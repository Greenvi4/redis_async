//
// Created by niko on 29.06.2021.
//
#include <redis_async/redis_async.hpp>

#include <redis_async/details/protocol/message.hpp>

#include <boost/lexical_cast.hpp>
#include <fstream>
#include <gtest/gtest.h>

#include "empty_port.hpp"
#include "test_server.hpp"

namespace ts = test_server;
namespace ep = empty_port;

TEST(CommandsTest, raw_cmd) {
    using redis_async::details::message;
    using redis_async::details::single_command_t;
    using redis_async::details::command_container_t;
    using Buffer = message::buffer_type;

    {
        single_command_t ping{"PING", "Hello, World!"};
        message m(ping);
        const std::string expected = "*2\r\n$4\r\nPING\r\n$13\r\nHello, World!\r\n";
        Buffer result(m.buffer().first, m.buffer().second);
        ASSERT_EQ(std::string(result.begin(), result.end()), expected);
    }
    {
        command_container_t cont = {{"PING", "Hello, World!"}, {"LPUSH", "list", "value"}};
        message m(cont);
        const std::string expected = "*2\r\n$4\r\nPING\r\n$13\r\nHello, World!\r\n"
                                     "*3\r\n$5\r\nLPUSH\r\n$4\r\nlist\r\n$5\r\nvalue\r\n";
        Buffer result(m.buffer().first, m.buffer().second);
        ASSERT_EQ(std::string(result.begin(), result.end()), expected);
    }
}

TEST(CommandsTest, ping) {
    uint16_t port = ep::get_random();
    auto port_str = boost::lexical_cast<std::string>(port);
    auto server = ts::make_server({"redis-server", "--port", port_str});
    ep::wait_port(port);

    using redis_async::rd_service;
    rd_service::add_connection("tcp=tcp://localhost:" + port_str, 1);

    boost::asio::deadline_timer timer(*rd_service::io_service(), boost::posix_time::seconds(5));
    timer.async_wait([&](const boost::system::error_code &ec) {
        if (ec)
            return;
        rd_service::stop();
        FAIL() << "Test timer expired";
    });

    using redis_async::result_t;

    rd_service::ping(
        "tcp"_rd,
        [](const result_t &res) { EXPECT_EQ("PONG", boost::get<redis_async::string_t>(res).str); },
        [&](const redis_async::error::rd_error &) {
            timer.cancel();
            rd_service::stop();
            FAIL();
        });

    const std::string msg = "Hello, World!";
    rd_service::ping(
        "tcp"_rd, msg,
        [msg](const result_t &res) { EXPECT_EQ(msg, boost::get<redis_async::string_t>(res).str); },
        [&](const redis_async::error::rd_error &) {
            timer.cancel();
            rd_service::stop();
            FAIL();
        });

    rd_service::echo(
        "tcp"_rd, msg,
        [&](const result_t &res) {
            EXPECT_EQ(msg, boost::get<redis_async::string_t>(res).str);
            timer.cancel();
            rd_service::stop();
        },
        [&](const redis_async::error::rd_error &) {
            timer.cancel();
            rd_service::stop();
            FAIL();
        });

    rd_service::run();
}