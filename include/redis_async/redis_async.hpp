//
// Created by niko on 23.05.2021.
//

#ifndef REDIS_ASYNC_REDIS_ASYNC_HPP
#define REDIS_ASYNC_REDIS_ASYNC_HPP

#include <redis_async/asio_config.hpp>
#include <redis_async/command_options.hpp>
#include <redis_async/commands.hpp>
#include <redis_async/common.hpp>

namespace redis_async {

    namespace details {
        struct redis_impl;
    } // namespace details

    class rd_service {
    public:
        static const size_t DEFAULT_POOL_SIZE = 4;

        /**
         *    @brief Add a connection specification.
         *
         *    Requires an alias, for the rd_service to be referenced by it later.
         *    @param connection_string
         *    @param pool_size A connection can have a pool size different from
         *             other connections.
         *    @throws redis_async::error::connection_error if the connection string
         *            cannot be used.
         */
        static void add_connection(std::string const &connection_string,
                                   optional_size pool_size = optional_size());

        static void add_connection(connection_options const &co,
                                   optional_size pool_size = optional_size());

        static void run();
        static void stop();

        static asio_config::io_service_ptr io_service();

        static void execute(rdalias &&alias, single_command_t &&cmd,
                             query_result_callback &&result, error_callback &&error);

    private:
        // No instances
        rd_service() = default;

        using pimpl = std::shared_ptr<details::redis_impl>;
        static pimpl &impl_ptr();
        static pimpl impl(size_t pool_size = DEFAULT_POOL_SIZE);
    };

} // namespace redis_async

#endif // REDIS_ASYNC_REDIS_ASYNC_HPP
