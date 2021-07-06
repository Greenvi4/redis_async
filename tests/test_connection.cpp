//
// Created by niko on 10.06.2021.
//

#include <boost/lexical_cast.hpp>
#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <redis_async/redis_async.hpp>

#include "empty_port.hpp"
#include "test_server.hpp"

namespace ts = test_server;
namespace ep = empty_port;

struct tmp_file_holder_t {
    char filename[PATH_MAX];

    tmp_file_holder_t()
        : filename{0}
        , m_fd(mkstemp(temp)) {
        assert(m_fd > 0);
        std::string path = "/proc/self/fd/" + std::to_string(m_fd);
        auto res = readlink(path.c_str(), filename, sizeof(filename));
        assert(res > 0);
    }

    ~tmp_file_holder_t() {
        close(m_fd);
        std::remove(filename);
    }

    tmp_file_holder_t(const tmp_file_holder_t &) = delete;
    tmp_file_holder_t& operator=(const tmp_file_holder_t &) = delete;

private:
    char temp[PATH_MAX] = "/tmp/redis.XXXXXX";
    int m_fd;
};

TEST(ConnectionTest, tcp) {
    uint16_t port = ep::get_random();
    auto port_str = boost::lexical_cast<std::string>(port);
    auto server = ts::make_server({"redis-server", "--port", port_str});
    ep::wait_port(port);

    using redis_async::rd_service;
    auto conn = redis_async::connection_options::parse("tcp=tcp://localhost:" + port_str);
    rd_service::add_connection(conn);
}

TEST(ConnectionTest, uds) {
    uint16_t port = ep::get_random();
    tmp_file_holder_t redis_config;
    tmp_file_holder_t redis_socket;
    {
        std::ofstream redis_out{redis_config.filename};
        redis_out << "port " << port << "\n";
        redis_out << "unixsocket " << redis_socket.filename << "\n";
    }
    auto server = ts::make_server({"redis-server", redis_config.filename});
    ep::wait_port(port);

    using redis_async::rd_service;
    rd_service::add_connection(std::string("uds=unix://") + redis_socket.filename);
}

TEST(ConnectionTest, conn_err) {
    uint16_t port = ep::get_random();
    auto port_str = boost::lexical_cast<std::string>(port);
    auto server = ts::make_server({"redis-server", "--port", port_str});
    ep::wait_port(port);

    using redis_async::rd_service;
    using redis_async::result_t;

    ASSERT_THROW(rd_service::ping(
                     "wrong_name_of_service"_rd, [](const result_t &) {},
                     [&](const redis_async::error::rd_error &err) { FAIL() << err.what(); }),
                 redis_async::error::connection_error);
    ASSERT_THROW(rd_service::add_connection("main=udp://192.168.0.10"_redis),
                 redis_async::error::connection_error);
    ASSERT_THROW(rd_service::add_connection("main=udp://192.168.0.10"_redis, 0),
                 redis_async::error::connection_error);
}
