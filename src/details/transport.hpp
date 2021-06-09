//
// Created by niko on 26.05.2021.
//

#ifndef REDIS_ASYNC_TRANSPORT_HPP
#define REDIS_ASYNC_TRANSPORT_HPP

#include <redis_async/asio_config.hpp>
#include <redis_async/common.hpp>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

namespace redis_async {
    namespace details {

        struct tcp_transport {
            using io_service_ptr = asio_config::io_service_ptr;
            using tcp = asio_config::tcp;
            using error_code = asio_config::error_code;
            using connect_callback = std::function<void(error_code const &)>;
            using socket_type = tcp::socket;

            explicit tcp_transport(const io_service_ptr &service);

            void connect_async(connection_options const &, connect_callback);
            bool connected() const;
            void close();

            template <typename BufferType, typename HandlerType>
            void async_read(BufferType &buffer, HandlerType handler) {
                boost::asio::async_read(socket, buffer, boost::asio::transfer_at_least(1), handler);
            }

            template <typename BufferType, typename HandlerType>
            void async_write(BufferType const &buffer, HandlerType handler) {
                boost::asio::async_write(socket, buffer, handler);
            }

        private:
            tcp::resolver resolver_;
            socket_type socket;

            void handle_resolve(error_code const &ec, tcp::resolver::iterator endpoint_iterator,
                                connect_callback);
            void handle_connect(error_code const &ec, connect_callback);
        };

        struct socket_transport {
            using io_service_ptr = asio_config::io_service_ptr;
            using socket_type = asio_config::stream_protocol::socket;
            using error_code = asio_config::error_code;
            using connect_callback = std::function<void(error_code const &)>;

            explicit socket_transport(const io_service_ptr &service);

            void connect_async(connection_options const &, const connect_callback&);
            bool connected() const;
            void close();

            template <typename BufferType, typename HandlerType>
            void async_read(BufferType &buffer, HandlerType handler) {
                boost::asio::async_read(socket, buffer, boost::asio::transfer_at_least(1), handler);
            }

            template <typename BufferType, typename HandlerType>
            void async_write(BufferType const &buffer, HandlerType handler) {
                boost::asio::async_write(socket, buffer, handler);
            }

        private:
            socket_type socket;
        };

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_TRANSPORT_HPP
