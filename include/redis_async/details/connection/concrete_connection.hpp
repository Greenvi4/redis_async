//
// Created by niko on 06.07.2021.
//

#ifndef REDIS_ASYNC_CONCRETE_CONNECTION_HPP
#define REDIS_ASYNC_CONCRETE_CONNECTION_HPP

#include <redis_async/details/connection/connection_fsm.hpp>

namespace redis_async {
    namespace details {

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

            ~concrete_connection() override {
                fsm_type::stop();
            };

        protected:
            void notifyIdleImpl() override {
                if (callbacks_.idle) {
                    callbacks_.idle(fsm_type::shared_from_this());
                } else {
                    LOG4CXX_WARN(logger,
                                 "Conn#" << fsm_type::number() << ": No connection idle callback")
                }
            }

            void notifyTerminatedImpl() override {
                if (callbacks_.terminated) {
                    callbacks_.terminated(fsm_type::shared_from_this());
                } else {
                    LOG4CXX_INFO(logger, "Conn#" << fsm_type::number()
                                                 << ": No connection terminated callback")
                }
                callbacks_ = connection_callbacks(); // clean up callbacks, no work further.
            }

            void notifyErrorImpl(error::connection_error const &e) override {
                LOG4CXX_ERROR(logger,
                              "Conn#" << fsm_type::number() << ": Connection error " << e.what())
                if (callbacks_.error) {
                    callbacks_.error(connection_ptr(), e);
                } else {
                    LOG4CXX_ERROR(logger,
                                  "Conn#" << fsm_type::number() << ": No connection_error callback")
                }
            }

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

#endif // REDIS_ASYNC_CONCRETE_CONNECTION_HPP
