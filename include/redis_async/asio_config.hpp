//
// Created by niko on 24.05.2021.
//

#ifndef REDIS_ASYNC_ASIO_CONFIG_HPP
#define REDIS_ASYNC_ASIO_CONFIG_HPP

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>

namespace redis_async {
    namespace asio_config {

        using io_service = boost::asio::io_service;
        using io_service_ptr = std::shared_ptr<io_service>;
        using tcp = boost::asio::ip::tcp;
        using stream_protocol = boost::asio::local::stream_protocol;
        using error_code = boost::system::error_code;

    } // namespace asio_config
} // namespace redis_async

#endif // REDIS_ASYNC_ASIO_CONFIG_HPP
