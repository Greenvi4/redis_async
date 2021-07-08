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
    namespace error = redis_async::error;
    namespace cmd = redis_async::cmd;
    using redis_async::result_t;

    rd_service::add_connection("tcp=tcp://localhost:" + port_str, 1);

    boost::asio::deadline_timer timer(*rd_service::io_service(), boost::posix_time::seconds(5));
    timer.async_wait([&](boost::system::error_code ec) {
        if (ec)
            return;
        rd_service::stop();
        FAIL() << "Test timer expired";
    });

    rd_service::execute(
        "tcp"_rd, cmd::ping(),
        [](const result_t &res) { EXPECT_EQ("PONG", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    const std::string msg = "Hello, World!";
    rd_service::execute(
        "tcp"_rd, cmd::ping(msg),
        [msg](const result_t &res) { EXPECT_EQ(msg, boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::ping(""),
        [&](const result_t &res) {
            EXPECT_EQ("", boost::get<redis_async::string_t>(res));
            timer.cancel();
            rd_service::stop();
        },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::run();
}

TEST(CommandsTest, echo) {
    uint16_t port = ep::get_random();
    auto port_str = boost::lexical_cast<std::string>(port);
    auto server = ts::make_server({"redis-server", "--port", port_str});
    ep::wait_port(port);

    using redis_async::rd_service;
    rd_service::add_connection("tcp=tcp://localhost:" + port_str, 1);

    boost::asio::deadline_timer timer(*rd_service::io_service(), boost::posix_time::seconds(5));
    timer.async_wait([&](boost::system::error_code ec) {
        if (ec)
            return;
        rd_service::stop();
        FAIL() << "Test timer expired";
    });

    using redis_async::result_t;
    namespace error = redis_async::error;
    namespace cmd = redis_async::cmd;
    const std::string msg = "Hello, World!";

    rd_service::execute(
        "tcp"_rd, cmd::echo(msg),
        [&](const result_t &res) { EXPECT_EQ(msg, boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::echo(""),
        [&](const result_t &res) {
            EXPECT_EQ("", boost::get<redis_async::string_t>(res));
            timer.cancel();
            rd_service::stop();
        },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::run();
}

TEST(CommandsTest, set_get) {
    uint16_t port = ep::get_random();
    auto port_str = boost::lexical_cast<std::string>(port);
    auto server = ts::make_server({"redis-server", "--port", port_str});
    ep::wait_port(port);

    using redis_async::rd_service;
    rd_service::add_connection("tcp=tcp://localhost:" + port_str, 1);

    boost::asio::deadline_timer timer(*rd_service::io_service(), boost::posix_time::seconds(5));
    timer.async_wait([&](boost::system::error_code ec) {
        if (ec)
            return;
        rd_service::stop();
        FAIL() << "Test timer expired";
    });

    using redis_async::result_t;
    namespace error = redis_async::error;
    namespace cmd = redis_async::cmd;

    const std::string key = "some_key";
    const std::string value = "some_value";

    rd_service::execute(
        "tcp"_rd, cmd::set(key, value),
        [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::set(key, value, redis_async::UpdateType::exist),
        [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::set(key, value, redis_async::UpdateType::not_exist),
        [&](const result_t &res) { EXPECT_NO_THROW(boost::get<redis_async::nil_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    ASSERT_THROW(
        rd_service::execute(
            "tcp"_rd, cmd::set(key, value, static_cast<redis_async::UpdateType>(100)),
            [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
            [&](const error::rd_error &err) {
                timer.cancel();
                rd_service::stop();
                FAIL() << err.what();
            }),
        error::client_error);

    using std::chrono::milliseconds;

    rd_service::execute(
        "tcp"_rd, cmd::set("expire_key_1", value, milliseconds(120)),
        [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::set("expire_key_2", value, milliseconds(0)),
        [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::set("expire_key_2", value, milliseconds(-100)),
        [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::get(key),
        [&](const result_t &res) { EXPECT_EQ(value, boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::get("another_key"),
        [&](const result_t &res) {
            EXPECT_NO_THROW(boost::get<redis_async::nil_t>(res));
            timer.cancel();
            rd_service::stop();
        },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::run();
}

TEST(CommandsTest, hadnle_exceptions) {

    uint16_t port = ep::get_random();
    auto port_str = boost::lexical_cast<std::string>(port);
    auto server = ts::make_server({"redis-server", "--port", port_str});
    ep::wait_port(port);

    using redis_async::rd_service;
    rd_service::add_connection("tcp=tcp://localhost:" + port_str, 1);

    boost::asio::deadline_timer timer(*rd_service::io_service(), boost::posix_time::seconds(5));
    timer.async_wait([&](boost::system::error_code ec) {
        if (ec)
            return;
        rd_service::stop();
        FAIL() << "Test timer expired";
    });

    using redis_async::result_t;
    namespace error = redis_async::error;
    namespace cmd = redis_async::cmd;

    rd_service::execute(
        "tcp"_rd, cmd::get("some_wrong_key"),
        [](const result_t &res) { boost::get<redis_async::string_t>(res); },
        [&](const error::rd_error &) {
            timer.cancel();
            rd_service::stop();
        });

    rd_service::run();
}
