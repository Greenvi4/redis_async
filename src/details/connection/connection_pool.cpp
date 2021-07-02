//
// Created by niko on 10.06.2021.
//

#include <redis_async/details/connection/base_connection.hpp>
#include <redis_async/details/connection/connection_pool.hpp>
#include <redis_async/details/connection/events.hpp>

#include <mutex>
#include <queue>
#include <utility>

namespace redis_async {
    namespace details {

        struct connection_pool::impl {
            using connections_container = ::std::vector<connection_ptr>;
            using connections_queue = ::std::queue<connection_ptr>;
            using request_callbacks_queue = ::std::queue<events::execute>;
            using mutex_type = ::std::recursive_mutex;
            using lock_type = ::std::lock_guard<mutex_type>;
            using atomic_flag = ::std::atomic_bool;

            io_service_ptr service_;
            size_t pool_size_;
            connection_options co_;
            mutex_type event_mutex_;
            mutex_type conn_mutex_;
            connections_container connections_;
            connections_queue ready_connections_;
            request_callbacks_queue queue_;
            atomic_flag closed_;
            simple_callback closed_callback_;

            impl(io_service_ptr service, size_t pool_size, connection_options co)
                : service_(std::move(service))
                , pool_size_(pool_size)
                , co_(std::move(co))
                , closed_(false) {
                if (pool_size_ == 0)
                    throw error::connection_error("Database connection pool size cannot be zero");

                if (co_.uri.empty())
                    throw error::connection_error("No URI in database connection string");

                LOG4CXX_INFO(logger, "Connection pool max size " << pool_size)
            }

            rdalias const &alias() const {
                return co_.alias;
            }

            //@{
            /** @name Connection granular work */
            bool get_idle_connection(connection_ptr &conn) {
                if (closed_)
                    return false;
                lock_type lock{conn_mutex_};
                if (!ready_connections_.empty()) {
                    conn = ready_connections_.front();
                    ready_connections_.pop();
                    return true;
                }
                return false;
            }
            void add_idle_connection(connection_ptr conn) {
                if (!closed_) {
                    lock_type lock{conn_mutex_};
                    ready_connections_.push(std::move(conn));
                    LOG4CXX_INFO(logger, alias()
                                             << " idle connections " << ready_connections_.size())
                }
            }
            void erase_connection(const connection_ptr &conn) {
                LOG4CXX_INFO(logger, "Erase connection from the connection pool")
                lock_type lock{conn_mutex_};
                auto f = std::find(connections_.begin(), connections_.end(), conn);
                if (f != connections_.end()) {
                    connections_.erase(f);
                }
            }
            //@}

            //@{
            /** @name Event queue */
            bool next_event(events::execute &evt) {
                lock_type lock{event_mutex_};
                if (!queue_.empty()) {
                    LOG4CXX_INFO(logger, alias() << " queue size " << queue_.size() << " (dequeue)")
                    evt = queue_.front();
                    queue_.pop();
                    return true;
                }
                return false;
            }

            void enqueue_event(events::execute &&evt) {
                lock_type lock{event_mutex_};
                queue_.push(::std::move(evt));
                LOG4CXX_INFO(logger, alias() << " queue size " << queue_.size() << " (enqueue)")
            }

            void clear_queue(error::connection_error const &ec) {
                lock_type lock(event_mutex_);
                while (!queue_.empty()) {
                    auto req = queue_.front();
                    queue_.pop();
                    if (req.error)
                        req.error(ec);
                }
            }
            //@}

            void create_new_connection(const connection_pool_ptr &pool) {
                if (closed_)
                    return;
                LOG4CXX_INFO(logger, "Create new " << alias() << " connection")
                connection_ptr conn = basic_connection::create(
                    service_, co_,
                    {[pool](connection_ptr c) { pool->connection_ready(c); },
                     [pool](connection_ptr c) { pool->connection_terminated(c); },
                     [pool](connection_ptr c, error::connection_error const &ec) {
                         pool->connection_error(c, ec);
                     }});

                {
                    lock_type lock{conn_mutex_};
                    connections_.push_back(conn);
                    LOG4CXX_INFO(logger, alias() << " pool size " << connections_.size())
                }
            }
            void connection_ready(connection_ptr c) {
                LOG4CXX_INFO(logger, "Connection " << alias() << " ready")

                events::execute evt;
                if (next_event(evt)) {
                    c->execute(::std::move(evt));
                } else {
                    if (closed_) {
                        close_connections();
                    } else {
                        add_idle_connection(c);
                    }
                }
            }
            void connection_terminated(connection_ptr c) {
                LOG4CXX_INFO(logger, "Connection " << alias() << " gracefully terminated")
                erase_connection(c);

                if (connections_.empty() && closed_ && closed_callback_) {
                    closed_callback_();
                }
                LOG4CXX_INFO(logger, alias() << " pool size " << connections_.size())
            }
            void connection_error(connection_ptr c, error::connection_error const &ec) {
                LOG4CXX_INFO(logger, "Connection " << alias() << " error: " << ec.what())
                erase_connection(c);
                clear_queue(ec);
            }
            void get_connection(command_wrapper_t cmd, query_result_callback const &conn_cb,
                                error_callback const &err, connection_pool_ptr pool) {
                if (closed_) {
                    err(error::connection_error("Connection pool is closed"));
                    return;
                }
                connection_ptr conn;
                if (get_idle_connection(conn)) {
                    LOG4CXX_INFO(logger, "Connection to " << alias() << " is idle")
                    conn->execute({std::move(cmd), conn_cb, err});
                } else {
                    if (!closed_ && connections_.size() < pool_size_) {
                        create_new_connection(pool);
                    }
                    enqueue_event({std::move(cmd), conn_cb, err});
                }
            }
            void close(simple_callback close_cb) {
                bool expected = false;
                if (closed_.compare_exchange_strong(expected, true)) {
                    closed_callback_ = close_cb;

                    lock_type lock{event_mutex_};
                    if (queue_.empty()) {
                        close_connections();
                    } else {
                        LOG4CXX_INFO(logger, "Wait for outstanding tasks to finish")
                    }
                }
            }
            void close_connections() {
                LOG4CXX_INFO(logger, "Close connection pool " << alias() << " pool size "
                                                              << connections_.size())
                if (!connections_.empty()) {
                    lock_type lock(conn_mutex_);
                    connections_container copy = connections_;
                    for (auto &c : copy) {
                        c->terminate();
                    }
                }
                if (closed_callback_) {
                    closed_callback_();
                }
            }
        };

        connection_pool::connection_pool(io_service_ptr service, size_t pool_size,
                                         connection_options const &co)
            : pimpl_(new impl(service, pool_size, co)) {
        }

        connection_pool::~connection_pool(){
            LOG4CXX_TRACE(logger, "*** connection_pool::~connection_pool()")}

        rdalias const &connection_pool::alias() const {
            return pimpl_->alias();
        }

        connection_pool::connection_pool_ptr connection_pool::create(io_service_ptr service,
                                                                     size_t pool_size,
                                                                     connection_options const &co) {
            connection_pool_ptr pool(new connection_pool(std::move(service), pool_size, co));
            pool->create_new_connection();
            return pool;
        }

        void connection_pool::create_new_connection() {
            auto _this = shared_from_this();
            pimpl_->create_new_connection(_this);
        }

        void connection_pool::connection_ready(connection_ptr c) {
            pimpl_->connection_ready(std::move(c));
        }

        void connection_pool::connection_terminated(connection_ptr c) {
            pimpl_->connection_terminated(std::move(c));
        }

        void connection_pool::connection_error(connection_ptr c,
                                               error::connection_error const &ec) {
            pimpl_->connection_error(std::move(c), ec);
        }

        void connection_pool::get_connection(command_wrapper_t cmd,
                                             query_result_callback const &conn_cb,
                                             error_callback const &err) {
            auto _this = shared_from_this();
            pimpl_->get_connection(std::move(cmd), conn_cb, err, _this);
        }

        void connection_pool::close(simple_callback close_cb) {
            pimpl_->close(std::move(close_cb));
        }

        void connection_pool::close_connections() {
            pimpl_->close_connections();
        }

    } // namespace details
} // namespace redis_async