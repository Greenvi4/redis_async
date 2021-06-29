//
// Created by niko on 10.06.2021.
//

#ifndef REDIS_ASYNC_REDIS_IMPL_HPP
#define REDIS_ASYNC_REDIS_IMPL_HPP

#include <redis_async/common.hpp>
#include <redis_async/asio_config.hpp>

#include <boost/noncopyable.hpp>
#include <map>

#include <details/protocol/command.hpp>

namespace redis_async {
    namespace details {

        struct connection_pool;

        class redis_impl : private boost::noncopyable {
            typedef std::shared_ptr<connection_pool> connection_pool_ptr;
            typedef std::map<rdalias, connection_pool_ptr> pools_map;

        public:
            explicit redis_impl(size_t pool_size);
            virtual ~redis_impl();

            void set_defaults(size_t pool_size);
            void add_connection(std::string const &connection_string,
                                optional_size pool_size = optional_size());
            void add_connection(const connection_options &options,
                                optional_size pool_size = optional_size());
            void get_connection(rdalias const &alias, command_wrapper_t cmd,
                                const query_result_callback &conn_cb, const error_callback &err);

            void run();
            void stop();

            asio_config::io_service_ptr io_service() {
                return service_;
            }

        private:
            connection_pool_ptr add_pool(const connection_options &co,
                                         optional_size pool_size = optional_size());

            asio_config::io_service_ptr service_;
            size_t pool_size_;
            pools_map connections_;

            enum state_type { running, closing, closed };
            state_type state_;
        };

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_REDIS_IMPL_HPP
