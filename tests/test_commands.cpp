//
// Created by niko on 29.06.2021.
//
#include <redis_async/redis_async.hpp>

#include <boost/lexical_cast.hpp>
#include <fstream>
#include <gtest/gtest.h>

#include "empty_port.hpp"
#include "test_server.hpp"

namespace ts = test_server;
namespace ep = empty_port;

TEST(CommandsTest, ping) {
    uint16_t port = ep::get_random();
    auto port_str = boost::lexical_cast<std::string>(port);
    auto server = ts::make_server({"redis-server", "--port", port_str});
    ep::wait_port(port);

    using redis_async::rd_service;
    rd_service::add_connection("tcp=tcp://localhost:" + port_str);

    using redis_async::resultset;

    rd_service::ping(
        "tcp"_rd, [](const resultset &res) { ASSERT_EQ(res, "PONG"); },
        [](const redis_async::error::rd_error &) { FAIL(); });

    const std::string msg = "Hello, World!";
    rd_service::ping(
        "tcp"_rd, msg,
        [msg](const resultset &res) {
            EXPECT_EQ(msg, res);
            rd_service::stop();
        },
        [](const redis_async::error::rd_error &) { FAIL(); });

    rd_service::echo(
        "tcp"_rd, msg,
        [msg](const resultset &res) {
          EXPECT_EQ(msg, res);
          rd_service::stop();
        },
        [](const redis_async::error::rd_error &) { FAIL(); });

    rd_service::run();
}