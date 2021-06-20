//
// Created by niko on 26.05.2021.
//

#include "transport.hpp"

#include <redis_async/error.hpp>

#include <boost/asio/connect.hpp>
#include <boost/bind.hpp>

namespace redis_async {
    namespace details {

        //****************************************************************************
        // tcp_layer
        tcp_transport::tcp_transport(const io_service_ptr &service)
            : resolver_(*service)
            , socket(*service) {
        }

        void tcp_transport::connect_async(connection_options const &conn, connect_callback cb) {
            if (conn.uri.empty()) {
                throw error::connection_error("No connection uri!");
            }
            if (conn.schema != "tcp") {
                throw error::connection_error("Wrong connection schema for TCP transport");
            }
            std::string host = conn.uri;
            std::string svc = "6379";
            std::string::size_type pos = conn.uri.find(':');
            if (pos != std::string::npos) {
                host = conn.uri.substr(0, pos);
                svc = conn.uri.substr(pos + 1);
            }

            tcp::resolver::query query(host, svc);
            resolver_.async_resolve(query,
                                    boost::bind(&tcp_transport::handle_resolve, this, _1, _2, cb));
        }

        void tcp_transport::handle_resolve(error_code const &ec,
                                           tcp::resolver::iterator endpoint_iterator,
                                           connect_callback cb) {
            if (!ec) {
                boost::asio::async_connect(
                    socket, std::move(endpoint_iterator),
                    boost::bind(&tcp_transport::handle_connect, this, _1, cb));
            } else {
                cb(ec);
            }
        }

        void tcp_transport::handle_connect(error_code const &ec, connect_callback cb) {
            cb(ec);
        }

        bool tcp_transport::connected() const {
            return socket.is_open();
        }

        void tcp_transport::close() {
            if (socket.is_open())
                socket.close();
        }

        //----------------------------------------------------------------------------
        // socket_transport implementation
        //----------------------------------------------------------------------------
        socket_transport::socket_transport(const io_service_ptr &service)
            : socket(*service) {
        }

        void socket_transport::connect_async(connection_options const &conn,
                                             const connect_callback &cb) {
            using asio_config::stream_protocol;
            if (conn.schema != "unix") {
                throw error::connection_error(
                    "Wrong connection schema for Unix Domain socket transport");
            }
            std::string uri = conn.uri;

            if (uri.empty()) {
                uri = "/tmp/.s.REDIS.6379";
            }
            socket.async_connect(stream_protocol::endpoint(uri), cb);
        }

        bool socket_transport::connected() const {
            return socket.is_open();
        }

        void socket_transport::close() {
            if (socket.is_open())
                socket.close();
        }

    } // namespace details
} // namespace redis_async