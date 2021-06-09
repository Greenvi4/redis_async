//
// Created by niko on 09.06.2021.
//

#ifndef REDIS_ASYNC_CONNECTION_FSM_HPP
#define REDIS_ASYNC_CONNECTION_FSM_HPP

#include <boost/asio/strand.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/state_machine_def.hpp>

#include <redis_async/asio_config.hpp>
#include <redis_async/common.hpp>
#include <redis_async/error.hpp>

#include "base_connection.hpp"
#include "events.hpp"

namespace redis_async {
    namespace details {

        namespace msm = boost::msm;
        namespace mpl = boost::mpl;

        template <typename TransportType, typename SharedType>
        struct connection_fsm_def
            : public msm::front::state_machine_def<connection_fsm_def<TransportType, SharedType>>,
              public std::enable_shared_from_this<SharedType> {

            using transport_type = TransportType;
            using shared_type = SharedType;
            using this_type = connection_fsm_def<transport_type, shared_type>;

            using connection_fsm_type = msm::back::state_machine<this_type>;
            using none = msm::front::none;
            using state = msm::front::state<>;
            using terminate_state = msm::front::terminate_state<>;

            template <typename SourceState, typename Event, typename TargetState,
                      typename Action = none, typename Guard = none>
            using tr = msm::front::Row<SourceState, Event, TargetState, Action, Guard>;

            //@{
            /** @name Actions */
            struct on_connection_error {
                template <typename SourceState, typename TargetState>
                void operator()(error::connection_error const &err, connection_fsm_type &fsm,
                                SourceState &, TargetState &) {
                    // fsm.log(logger::ERROR)
                    // << "connection::on_connection_error Error: " <<
                    // err.what();
                    // fsm.notify_error(err);
                }
            };
            struct disconnect {
                template <typename SourceState, typename TargetState>
                void operator()(events::terminate const &, connection_fsm_type &fsm, SourceState &,
                                TargetState &) {
                    // fsm.log() << "connection: disconnect";
                    // fsm.close_transport();
                }
            };
            //@}

            struct unplugged : state {};
            struct terminated : terminate_state {};
            struct connected : state {};
            struct authn : state {};
            struct idle : state {};
            struct query : state {};

            using initial_state = unplugged;

            // clang-format off
            using transaction_table = mpl::vector<
                /*  Start        Event                       Next        Action               */
                /*+------------+---------------------------+-----------+---------------------+*/
                tr<unplugged,   connection_options,         connected,  none>,
                tr<unplugged,   events::terminate,          terminated, none>,

                tr<connected,   events::complete,           authn,      none>,
                tr<connected,   error::connection_error,    terminated, on_connection_error>,

                tr<authn,       events::ready_for_query,    idle,       none>,
                tr<authn,       error::connection_error,    terminated, on_connection_error>,

                tr<idle,        events::execute,            query,      none>,
                tr<idle,        events::terminate,          terminated, disconnect>,
                tr<idle,        error::connection_error,    terminated, on_connection_error>,

                tr<query,       events::ready_for_query,    idle,       none>,
                tr<query,       error::connection_error,    terminated, on_connection_error>
            >;
            // clang-format on

            //@{
            using io_service_ptr = asio_config::io_service_ptr;
            using shared_base = std::enable_shared_from_this<shared_type>;
            //@}

            //@{
            connection_fsm_def(io_service_ptr svc)
                : shared_base()
                , io_service_{svc}
                , strand_{*svc}
                , transport_{svc}
                , connection_number_{next_connection_number()} {
                incoming_.prepare(8192); // FIXME Magic number, move to configuration
            }
            virtual ~connection_fsm_def() {
            }
            //@}

            size_t number() const {
                return connection_number_;
            }

            connection_options const &options() const {
                return conn_opts_;
            }

            static size_t next_connection_number() {
                static std::atomic<size_t> _number{0};
                return _number++;
            }

        private:
            asio_config::io_service_ptr io_service_;
            asio_config::io_service::strand strand_;
            transport_type transport_;
            boost::asio::streambuf incoming_;
            size_t connection_number_;

        protected:
            connection_options conn_opts_;
        };

        template <typename TransportType>
        class concrete_connection
            : public basic_connection,
              public msm::back::state_machine<
                  connection_fsm_def<TransportType, concrete_connection<TransportType>>> {
        public:
            using transport_type = TransportType;
            using this_type = concrete_connection<transport_type>;
            using fsm_type = msm::back::state_machine<connection_fsm_def<TransportType, this_type>>;

            concrete_connection(const io_service_ptr &svc, connection_callbacks callbacks)
                : basic_connection()
                , fsm_type(svc)
                , callbacks_(std::move(callbacks)) {
            }

            virtual ~concrete_connection() override = default;

        private:
            void connectImpl(connection_options const &opts) override {
                fsm_type::process_event(opts);
            }

            rdalias const &aliasImpl() const override {
                return fsm_type::conn_opts_.alias;
            }

            void executeImpl(events::execute &&query) override {
                fsm_type::process_event(::std::move(query));
            }

            void terminateImpl() override {
                fsm_type::process_event(events::terminate{});
            }

        private:
            connection_callbacks callbacks_;
        };

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_CONNECTION_FSM_HPP
