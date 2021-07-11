//
// Created by niko on 11.07.2021.
//

#ifndef REDIS_ASYNC_REDIS_INSTANCE_HPP
#define REDIS_ASYNC_REDIS_INSTANCE_HPP

#include "empty_port.hpp"
#include "test_server.hpp"

namespace redis_async {
    namespace test {

        class RedisInstance {
        public:
            explicit RedisInstance();
            std::string getUri() const;

        private:
            using Server = test_server::TestServer;
            using ServerPtr = std::unique_ptr<Server>;

            ServerPtr m_server;
            std::string m_uri;
        };

        RedisInstance::RedisInstance() {
            namespace ts = test_server;
            namespace ep = empty_port;

            auto port = ep::get_random();
            auto port_str = boost::lexical_cast<std::string>(port);
            m_server = ts::make_server({"redis-server", "--port", port_str});
            ep::wait_port(port);
            m_uri = "tcp://localhost:" + port_str;
        }

        std::string RedisInstance::getUri() const {
            return m_uri;
        }

    } // namespace test
} // namespace redis_async

#endif // REDIS_ASYNC_REDIS_INSTANCE_HPP
