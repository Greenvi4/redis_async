//
// Created by niko on 10.06.2021.
//

#ifndef REDIS_ASYNC_CONNECTION_POOL_HPP
#define REDIS_ASYNC_CONNECTION_POOL_HPP

#include <boost/noncopyable.hpp>
#include <memory>

#include <redis_async/asio_config.hpp>
#include <redis_async/common.hpp>
#include <redis_async/details/protocol/command.hpp>

namespace redis_async {
    namespace details {

        /**
         * Container of connections to the same database
         */
        class connection_pool : public ::std::enable_shared_from_this<connection_pool>,
                                private boost::noncopyable {
        public:
            using io_service_ptr = asio_config::io_service_ptr;
            using connection_pool_ptr = ::std::shared_ptr<connection_pool>;

        public:
            static connection_pool_ptr create(io_service_ptr service, size_t pool_size,
                                              connection_options const &co);

            ~connection_pool();

            rdalias const &alias() const;
            void get_connection(command_wrapper_t cmd, const query_result_callback &conn_cb,
                                const error_callback &err);
            void close(simple_callback);

        private:
            connection_pool(io_service_ptr service, size_t pool_size, connection_options const &co);

            void create_new_connection();
            void connection_ready(connection_ptr c);
            void connection_terminated(connection_ptr c);
            void connection_error(connection_ptr c, error::connection_error const &ec);
            void close_connections();

        private:
            struct impl;
            using pimpl = ::std::unique_ptr<impl>;
            pimpl pimpl_;
        };

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_CONNECTION_POOL_HPP
