//
// Created by niko on 23.05.2021.
//

#ifndef REDIS_ASYNC_REDIS_ASYNC_HPP
#define REDIS_ASYNC_REDIS_ASYNC_HPP

#include <redis_async/asio_config.hpp>
#include <redis_async/common.hpp>

#include <boost/utility/string_ref.hpp>

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

        /**
         *     @brief Create a connection or retrieve a connection from the connection pool
         *         and start a transaction.
         *
         *    Will lookup a connection by alias. If a new connection must be created,
         *     it will be created with the connection string associated with the alias.
         *    If there is an idle connection in the pool, will return it.
         *    If no idle connections are available, and the size of connection pool
         *    didn't reach it's limit, will create a new one.
         *    If the pool is full and no idle connections are available,
         *    will return the first connection that becomes idle.
         *
         *    @param alias database alias
         *    @param result callback function that will be called when a connection
         *             becomes available and transaction is started.
         *    @param error callback function that will be called in case of an error.
         *    @param isolation transaction isolation level
         *    @throws tip::db::pg::error::connection_error if the alias is not
         *          registered with the database service.
         */
        static void ping(const rdalias &alias, const query_result_callback &result,
                         const error_callback &error);
        static void ping(const rdalias &alias, boost::string_ref msg,
                         const query_result_callback &result, const error_callback &error);
        static void echo(const rdalias &alias, boost::string_ref msg,
                         const query_result_callback &result, const error_callback &error);
        static void set(const rdalias &alias, boost::string_ref key, boost::string_ref value,
                         const query_result_callback &result, const error_callback &error);
        static void get(const rdalias &alias, boost::string_ref key,
                        const query_result_callback &result, const error_callback &error);


    private:
        // No instances
        rd_service() = default;

        using pimpl = std::shared_ptr<details::redis_impl>;
        static pimpl &impl_ptr();
        static pimpl impl(size_t pool_size = DEFAULT_POOL_SIZE);
    };

} // namespace redis_async

#endif // REDIS_ASYNC_REDIS_ASYNC_HPP
