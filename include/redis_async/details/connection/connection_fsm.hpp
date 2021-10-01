//
// Created by niko on 09.06.2021.
//

#ifndef REDIS_ASYNC_CONNECTION_FSM_HPP
#define REDIS_ASYNC_CONNECTION_FSM_HPP

#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/state_machine_def.hpp>

#include <redis_async/asio_config.hpp>
#include <redis_async/common.hpp>
#include <redis_async/details/connection/base_connection.hpp>
#include <redis_async/details/connection/events.hpp>
#include <redis_async/details/connection/handler_parse_result.hpp>
#include <redis_async/details/protocol/parser.hpp>
#include <redis_async/details/protocol/serializer.hpp>
#include <redis_async/error.hpp>

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

            using buffer = boost::asio::streambuf;
            using iterator = boost::asio::buffers_iterator<buffer::const_buffers_type, char>;

            template <typename SourceState, typename Event, typename TargetState,
                      typename Action = none, typename Guard = none>
            using tr = msm::front::Row<SourceState, Event, TargetState, Action, Guard>;

            //@{
            /** @name Actions */
            struct on_connection_error {
                template <typename SourceState, typename TargetState>
                void operator()(error::connection_error const &err, connection_fsm_type &fsm,
                                SourceState &, TargetState &) {
                    LOG4CXX_ERROR(logger, "Conn#" << fsm.number()
                                                  << ": connection::on_connection_error Error: "
                                                  << err.what())
                    fsm.notify_error(err);
                }
            };

            struct disconnect {
                template <typename SourceState, typename TargetState>
                void operator()(events::terminate, connection_fsm_type &fsm, SourceState &,
                                TargetState &) {
                    LOG4CXX_INFO(logger, "Conn#" << fsm.number() << ": connection: disconnect")
                    fsm.close_transport();
                }
            };
            //@}

            struct unplugged : state {
                // clang-format off
                using deferred_events = mpl::vector<
                    events::terminate,
                    events::execute
                    >;
                // clang-format on

                template <typename Event>
                void on_entry(Event const &, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger, "Conn#" << fsm.number() << ": state[unplugged]: entry")
                }

                template <typename Event>
                void on_exit(Event const &, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger, "Conn#" << fsm.number() << ": state[unplugged]: exit")
                }
            };

            struct terminated : terminate_state {
                template <typename Event>
                void on_entry(Event const &, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger, "Conn#" << fsm.number() << ": state[terminated]: entry")
                    fsm.notify_terminated();
                }
            };

            struct connecting : state {
                // clang-format off
                using deferred_events = mpl::vector<
                    events::terminate,
                    events::execute
                >;
                // clang-format on

                void on_entry(connection_options const &opts, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger, "Conn#" << fsm.number() << ": state[connecting]: entry")
                    fsm.connect_transport(opts);
                }

                void on_exit(events::complete, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger, "Conn#" << fsm.number()
                                                  << ": state[connecting]: exit by complete")
                    fsm.start_read();
                }

                template <typename Event>
                void on_exit(Event const &, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger, "Conn#" << fsm.number() << ": state[connecting]: exit by "
                                                  << demangle<Event>())
                }
            };

            struct authn : state {
                // clang-format off
                using deferred_events = mpl::vector<
                    events::terminate,
                    events::execute
                >;
                // clang-format on

                template <typename Event>
                void on_entry(Event const &, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger, "Conn#" << fsm.number() << ": state[authn]: entry")
                    fsm.send_startup_message();
                }

                void on_exit(const events::recv &evt, connection_fsm_type &fsm) {
                    boost::ignore_unused_variable_warning(evt);
                    LOG4CXX_TRACE(logger,
                                  "Conn#" << fsm.number() << ": state[authn]: exit by recv");
                    //! @todo check answer
                }

                template <typename Event>
                void on_exit(Event const &, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger, "Conn#" << fsm.number() << ": state[authn]: exit by "
                                                  << demangle<Event>());
                }
            };

            struct idle : state {
                template <typename Event>
                void on_entry(Event const &, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger, "Conn#" << fsm.number() << ": state[idle]: entry")
                    fsm.notify_idle();
                }
            };

            struct query : state {
                // clang-format off
                using deferred_events = mpl::vector<
                    events::terminate
                >;
                // clang-format on

                events::execute query_;

                void on_entry(const events::execute &evt, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger,
                                  "Conn#" << fsm.number() << ": state[query]: entry by execute")
                    query_ = evt;
                    fsm.send(std::move(query_.buff));
                }

                template <typename Event>
                void on_entry(Event const &, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger, "Conn#" << fsm.number() << ": state[query]: entry")
                    query_ = events::execute{};
                }

                void on_exit(const error::query_error &err, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger,
                                  "Conn#" << fsm.number() << ": state[query]: exit by query_error");
                    fsm.notify_error(*this, err);
                    query_ = events::execute{};
                }

                void on_exit(const events::recv &evt, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger,
                                  "Conn#" << fsm.number() << ": state[query]: exit by recv");
                    fsm.notify_result(*this, evt.res);
                    query_ = events::execute{};
                }

                template <typename Event>
                void on_exit(Event const &, connection_fsm_type &fsm) {
                    LOG4CXX_TRACE(logger, "Conn#" << fsm.number() << ": state[query]: exit by "
                                                  << demangle<Event>())
                    query_ = events::execute{};
                }
            };

            using initial_state = unplugged;

            // clang-format off
            using transition_table = mpl::vector<
                /*  Start        Event                       Next        Action               */
                /*+------------+---------------------------+-----------+---------------------+*/
                tr<unplugged,   connection_options,         connecting, none>,
                tr<unplugged,   events::terminate,          terminated, none>,

                tr<connecting,  events::complete,           authn,      none>,
                tr<connecting,  error::connection_error,    terminated, on_connection_error>,

                tr<authn,       events::complete,           idle,       none>,
                tr<authn,       events::recv,               idle,       none>,
                tr<authn,       error::connection_error,    terminated, on_connection_error>,

                tr<idle,        events::execute,            query,      none>,
                tr<idle,        events::terminate,          terminated, disconnect>,
                tr<idle,        error::connection_error,    terminated, on_connection_error>,

                tr<query,       events::recv,               idle,       none>,
                tr<query,       error::query_error,         idle,       none>,
                tr<query,       error::connection_error,    terminated, on_connection_error>
            >;
            // clang-format on

            // Replaces the default no-transition response.
            template <class FSM, class Event>
            void no_transition(Event const &e, FSM &fsm, int state) {
                boost::ignore_unused_variable_warning(e);
                LOG4CXX_ERROR(logger, "Conn#" << fsm.number() << ": no transition from state "
                                              << state << " on event " << demangle<Event>());
                BOOST_ASSERT(false);
            }

            //@{
            using io_service_ptr = asio_config::io_service_ptr;
            using shared_base = std::enable_shared_from_this<shared_type>;
            using asio_io_handler =
                std::function<void(asio_config::error_code const &error, size_t bytes_transferred)>;
            //@}

            //@{
            explicit connection_fsm_def(io_service_ptr svc)
                : shared_base()
                , io_service_{svc}
                , strand_{*svc}
                , transport_{svc}
                , connection_number_{next_connection_number()} {
                incoming_.prepare(8192); // FIXME Magic number, move to configuration
            }

            virtual ~connection_fsm_def() = default;
            //@}

            size_t number() const {
                return connection_number_;
            }

            void connect_transport(connection_options const &opts) {
                if (opts.uri.empty()) {
                    throw error::connection_error("No connection uri!");
                }

                conn_opts_ = opts;
                auto _this = shared_base::shared_from_this();
                transport_.connect_async(
                    conn_opts_, [_this](asio_config::error_code ec) { _this->handle_connect(ec); });
            }

            void start_read() {
                auto _this = shared_base::shared_from_this();
                transport_.async_read(
                    incoming_, [_this](asio_config::error_code ec, size_t bytes_transferred) {
                        _this->handle_read(ec, bytes_transferred);
                    });
            }

            void send_startup_message() {
                if (conn_opts_.password.empty()) {
                    fsm().process_event(events::complete{});
                    return;
                }
                single_command_t cmd{"AUTH", conn_opts_.password};
                events::execute::Buffer buff;
                Protocol::serialize(buff, cmd);
                send(std::move(buff));
            }

            void send(events::execute::Buffer &&m,
                      const asio_io_handler &handler = asio_io_handler()) {
                if (transport_.connected()) {
                    auto msg = ::std::make_shared<events::execute::Buffer>(::std::move(m));
                    auto _this = shared_base::shared_from_this();
                    auto write_handler = [_this, handler, msg](asio_config::error_code ec,
                                                               size_t sz) {
                        if (handler)
                            handler(ec, sz);
                        else
                            _this->handle_write(ec, sz);
                    };
                    transport_.async_write(boost::asio::buffer(msg->data(), msg->size()),
                                           write_handler);
                }
            }

            void close_transport() {
                transport_.close();
            }

            //@{
            /** @connection events notifications */
            template <typename Source>
            void notify_result(Source &state, const result_t &res) {
                if (state.query_.result) {
                    auto result_cb = state.query_.result;
                    auto error_cb = state.query_.error;
                    auto conn = fsm().shared_from_this();
                    fsm().async_notify([conn, result_cb, error_cb, res]() {
                        LOG4CXX_TRACE(logger, "Conn#" << conn->number() << ": In async notify");
                        try {
                            result_cb(res);
                        } catch (error::query_error const &e) {
                            LOG4CXX_TRACE(
                                logger, "Conn#" << conn->number()
                                                << ": Query result handler throwed a query_error: "
                                                << e.what());
                            error_cb(e);
                        } catch (error::rd_error const &e) {
                            LOG4CXX_TRACE(logger,
                                          "Conn#" << conn->number()
                                                  << ": Query result handler throwed a db_error: "
                                                  << e.what());
                            error_cb(e);
                        } catch (std::exception const &e) {
                            LOG4CXX_TRACE(logger,
                                          "Conn#" << conn->number()
                                                  << ": Query result handler throwed an exception: "
                                                  << e.what());
                            error_cb(error::client_error(e));
                        } catch (...) {
                            LOG4CXX_TRACE(
                                logger,
                                "Conn#" << conn->number()
                                        << ": Query result handler throwed an unknown exception");
                            error_cb(error::client_error("Unknown exception"));
                        }
                    });
                }
            }

            void notify_idle() {
                try {
                    notifyIdleImpl();
                } catch (::std::exception const &e) {
                    LOG4CXX_WARN(logger, "Conn#" << number() << ": Exception in on idle handler "
                                                 << e.what())
                } catch (...) {
                    // Ignore handler error
                    LOG4CXX_WARN(logger, "Conn#" << number() << ": Exception in on idle handler")
                }
            }

            void notify_terminated() {
                try {
                    notifyTerminatedImpl();
                } catch (::std::exception const &e) {
                    LOG4CXX_WARN(logger, "Conn#" << number() << ": Exception in terminated handler "
                                                 << e.what())
                } catch (...) {
                    // Ignore handler error
                    LOG4CXX_WARN(logger, "Conn#" << number() << ": Exception in terminated handler")
                }
            }

            template <typename State>
            void notify_error(State &state, error::query_error const &qe) {
                if (state.query_.error) {
                    try {
                        state.query_.error(qe);
                    } catch (std::exception const &e) {
                        LOG4CXX_WARN(logger,
                                     "Query error handler throwed an exception: " << e.what());
                    } catch (...) {
                        LOG4CXX_WARN(logger, "Query error handler throwed an unexpected exception");
                    }
                } else {
                    LOG4CXX_WARN(logger, "No query error handler");
                }
            }

            void notify_error(error::connection_error const &e) {
                notifyErrorImpl(e);
            }

            template <typename Handler>
            void async_notify(Handler &&h) {
                strand_.post(::std::forward<Handler>(h));
            }
            //@}

            connection_options const &options() const {
                return conn_opts_;
            }

            static size_t next_connection_number() {
                static std::atomic<size_t> _number{0};
                return _number++;
            }

        protected:
            // clang-format off
            virtual void notifyIdleImpl() {}
            virtual void notifyTerminatedImpl() {}
            virtual void notifyErrorImpl(error::connection_error const &) {}
            // clang-format on

        protected:
            connection_options conn_opts_;

        private:
            connection_fsm_type &fsm() {
                return static_cast<connection_fsm_type &>(*this);
            }

            connection_fsm_type const &fsm() const {
                return static_cast<connection_fsm_type const &>(*this);
            }

            void handle_connect(asio_config::error_code ec) {
                if (!ec) {
                    fsm().process_event(events::complete{});
                } else {
                    fsm().process_event(error::connection_error{ec.message()});
                }
            }

            void handle_read(asio_config::error_code ec, size_t bytes_transferred) {
                if (!ec) {
                    // read message
                    read_message(bytes_transferred);
                    // start async operation again
                    start_read();
                } else {
                    // Socket error - force termination
                    fsm().process_event(error::connection_error(ec.message()));
                }
            }

            void handle_write(asio_config::error_code ec, size_t) {
                if (ec) {
                    // Socket error - force termination
                    fsm().process_event(error::connection_error(ec.message()));
                }
            }

            void read_message(size_t max_bytes) {
                while (max_bytes) {
                    using handler_t = handler_parse_result_t<connection_fsm_type>;
                    auto data = incoming_.data();
                    auto parsed_result =
                        redis_async::details::raw_parse(iterator::begin(data), iterator::end(data));
                    auto consumed = std::visit(handler_t{fsm()}, parsed_result);
                    if (!consumed)
                        consumed = max_bytes;
                    incoming_.consume(consumed);
                    max_bytes -= consumed;
                }
            }

        private:
            asio_config::io_service_ptr io_service_;
            asio_config::io_service::strand strand_;
            transport_type transport_;
            buffer incoming_;
            size_t connection_number_;
        };

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_CONNECTION_FSM_HPP
