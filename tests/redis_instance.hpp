//
// Created by niko on 11.07.2021.
//

#ifndef REDIS_ASYNC_REDIS_INSTANCE_HPP
#define REDIS_ASYNC_REDIS_INSTANCE_HPP

#include <redis_async/redis_async.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <list>
#include <utility>

#include "empty_port.hpp"
#include "test_server.hpp"

namespace redis_async {
    namespace test {

        namespace instance {

            class Server : boost::noncopyable {
            public:
                using Redis = test_server::TestServer;
                using RedisPtr = std::unique_ptr<Redis>;

                explicit Server();
                const std::string &getUri() const;

            private:
                RedisPtr m_server;
                std::string m_uri;
            };

            class Client {
            public:
                using optional_size = redis_async::optional_size;
                using timer = boost::asio::deadline_timer;
                using duration_type = timer::duration_type;

                explicit Client();
                ~Client();

                void run();

                const std::string &getUri() const;
                void add_connection(const std::string &name, optional_size pool_size);

                template <typename WaitHandler>
                BOOST_ASIO_INITFN_RESULT_TYPE(WaitHandler, void(boost::system::error_code))
                add_deadline_timer(const duration_type &expiry_time,
                                   BOOST_ASIO_MOVE_ARG(WaitHandler) handler) {
                    m_timers.emplace_back(*rd_service::io_service(), expiry_time);
                    m_timers.back().async_wait(std::forward<WaitHandler>(handler));
                }

            private:
                using Server = instance::Server;
                using ServerPtr = std::unique_ptr<Server>;
                using Timers = std::list<timer>;

                Timers m_timers;
                ServerPtr m_server;
            };

            inline Server::Server() {
                namespace ts = test_server;
                namespace ep = empty_port;

                auto port = ep::get_random();
                auto port_str = boost::lexical_cast<std::string>(port);
                m_server = ts::make_server({"redis-server", "--port", port_str});
                m_uri = "tcp://localhost:" + port_str;
                ep::wait_port(port);
            }

            inline const std::string &Server::getUri() const {
                return m_uri;
            }

            inline Client::Client()
                : m_server(std::make_unique<Server>()) {
            }

            inline Client::~Client() {
                using redis_async::rd_service;
                for (auto &timer : m_timers)
                    timer.cancel();
                rd_service::stop();
            }

            inline const std::string &Client::getUri() const {
                return m_server->getUri();
            }

            inline void Client::add_connection(const std::string &name, optional_size pool_size) {
                using redis_async::rd_service;
                rd_service::add_connection(name + "=" + getUri(), std::move(pool_size));
            }

            inline void Client::run() {
                using redis_async::rd_service;
                rd_service::run();
            }

        } // namespace instance

    } // namespace test
} // namespace redis_async

#endif // REDIS_ASYNC_REDIS_INSTANCE_HPP
