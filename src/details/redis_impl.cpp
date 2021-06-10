//
// Created by niko on 10.06.2021.
//

#include "redis_impl.hpp"

#include <utility>

#include "base_connection.hpp"
#include "connection_pool.hpp"

namespace redis_async {
    namespace details {

        redis_impl::redis_impl(size_t pool_size)
            : service_(std::make_shared<asio_config::io_service>())
            , pool_size_(pool_size)
            , state_(running) {
            LOG4CXX_TRACE(logger, "Initializing rd_service db service");
        }

        redis_impl::~redis_impl() {
            stop();
        }

        void redis_impl::set_defaults(size_t pool_size) {
            pool_size_ = pool_size;
        }

        void redis_impl::add_connection(const std::string &connection_string,
                                        optional_size pool_size) {
            if (state_ != running)
                throw error::connection_error("Database service is not running");
            connection_options co = connection_options::parse(connection_string);
            add_connection(co, std::move(pool_size));
        }

        void redis_impl::add_connection(const connection_options &options,
                                        optional_size pool_size) {
            if (state_ != running)
                throw error::connection_error("Database service is not running");

            if (options.uri.empty())
                throw error::connection_error("No URI in database connection string");

            if (options.database.empty())
                throw error::connection_error("No database name in database connection string");

            if (options.alias.empty())
                throw error::connection_error("No alias name in database connection string");

            add_pool(options, std::move(pool_size));
        }

        void redis_impl::get_connection(rdalias const &alias, const std::string &expression,
                                        const query_result_callback &conn_cb,
                                        const error_callback &err) {
            if (state_ != running)
                throw error::connection_error("Database service is not running");

            if (!connections_.count(alias)) {
                throw error::connection_error("Database alias '" + alias + "' is not registered");
            }
            connection_pool_ptr pool = connections_[alias];
            pool->get_connection(expression, conn_cb, err);
        }

        void redis_impl::run() {
            service_->run();
        }

        void redis_impl::stop() {
            if (state_ == running) {
                state_ = closing;
                std::shared_ptr<size_t> pool_count = std::make_shared<size_t>(connections_.size());
                asio_config::io_service_ptr svc = service_;

                for (auto &c : connections_) {
                    // Pass a close callback. Call stop
                    // only when all connections are closed, may be with some timeout
                    c.second->close([pool_count, svc]() {
                        --(*pool_count);
                        if (*pool_count == 0) {
                            svc->stop();
                        }
                    });
                }
                connections_.clear();
            }
        }

        redis_impl::connection_pool_ptr redis_impl::add_pool(const connection_options &co,
                                                             optional_size pool_size) {
            if (!connections_.count(co.alias)) {
                if (!pool_size.is_initialized()) {
                    pool_size = pool_size_;
                }
                LOG4CXX_INFO(logger,
                             "Create a new connection pool " << co.alias << " size " << *pool_size);
                LOG4CXX_INFO(logger, "Register new connection " << co.uri << "[" << co.database
                                                                << "]"
                                                                << " with alias " << co.alias);
                connections_.insert(std::make_pair(
                    co.alias,
                    connection_pool_ptr(connection_pool::create(service_, *pool_size, co))));
            }
            return connections_[co.alias];
        }

    } // namespace details
} // namespace redis_async
