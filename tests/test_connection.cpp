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
    rd_service::add_connection("tcp=tcp://localhost:" + port_str);
//    rd_service::run();
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
//    rd_service::run();
}
