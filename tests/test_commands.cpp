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

TEST(CommandsTest, keys) {
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
        "tcp"_rd, cmd::mset({{"key1", "value1"}, {"key2", "value2"}, {"key3", "value3"}}),
        [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::del({"key3", "nokey"}),
        [&](const result_t &res) { EXPECT_EQ(1, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::exists({"key2", "nokey"}),
        [&](const result_t &res) { EXPECT_EQ(1, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::exists({"key1", "key2"}),
        [&](const result_t &res) { EXPECT_EQ(2, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::expire("key1", std::chrono::seconds(10)),
        [&](const result_t &res) { EXPECT_EQ(1, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::ttl("key1"),
        [&](const result_t &res) { EXPECT_EQ(10, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::pexpire("key2", std::chrono::seconds(1)),
        [&](const result_t &res) { EXPECT_EQ(1, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::pttl("key2"),
        [&](const result_t &res) { EXPECT_EQ(1000, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::rename("key1", "another_key1"),
        [&](const result_t &res) {
            EXPECT_EQ("OK", boost::get<redis_async::string_t>(res));
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

TEST(CommandsTest, mset_mget) {
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
        "tcp"_rd, cmd::mset({{"key1", "value1"}, {"key2", "value2"}, {"key3", "value3"}}),
        [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::mset({{"key1", ""}, {"key2", ""}, {"key3", ""}}),
        [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::mget({"key1", "key2", "key3"}),
        [&](const result_t &res) {
            auto values = boost::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 3);
            EXPECT_EQ("", boost::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("", boost::get<redis_async::string_t>(values.elements[1]));
            EXPECT_EQ("", boost::get<redis_async::string_t>(values.elements[2]));
            timer.cancel();
            rd_service::stop();
        },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    ASSERT_THROW(
        rd_service::execute(
            "tcp"_rd, cmd::mset({}),
            [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
            [&](const error::rd_error &err) {
                timer.cancel();
                rd_service::stop();
                FAIL() << err.what();
            }),
        error::client_error);

    ASSERT_THROW(rd_service::execute(
                     "tcp"_rd, cmd::mget({}), [&](const result_t &res) {},
                     [&](const error::rd_error &err) {
                         timer.cancel();
                         rd_service::stop();
                         FAIL() << err.what();
                     }),
                 error::client_error);

    rd_service::run();
}

TEST(CommandsTest, hash) {
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
        "tcp"_rd, cmd::hset("hash", {{"key1", "value1"}, {"key2", "value2"}, {"key3", "value3"}}),
        [&](const result_t &res) { EXPECT_EQ(3, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::hget("hash", "key1"),
        [&](const result_t &res) { EXPECT_EQ("value1", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::hget("hash", "key2"),
        [&](const result_t &res) { EXPECT_EQ("value2", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::hdel("hash", {"key3", "does_not_exists"}),
        [&](const result_t &res) { EXPECT_EQ(1, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::hget("hash", "key3"),
        [&](const result_t &res) { EXPECT_NO_THROW(boost::get<redis_async::nil_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::hkeys("hash"),
        [&](const result_t &res) {
            auto values = boost::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 2);
            EXPECT_EQ("key1", boost::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("key2", boost::get<redis_async::string_t>(values.elements[1]));
        },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd,
        cmd::hmset(
            "hash",
            {{"key1", "another_value1"}, {"key2", "another_value2"}, {"key3", "another_value3"}}),
        [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::hmget("hash", {"key1", "key2", "key3", "doe_not_exists"}),
        [&](const result_t &res) {
            auto values = boost::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 4);
            EXPECT_EQ("another_value1", boost::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("another_value2", boost::get<redis_async::string_t>(values.elements[1]));
            EXPECT_EQ("another_value3", boost::get<redis_async::string_t>(values.elements[2]));
            EXPECT_NO_THROW(boost::get<redis_async::nil_t>(values.elements[3]));
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

TEST(CommandsTest, lists) {
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
        "tcp"_rd, cmd::lpush("list1", {"value1", "value2", "value3"}),
        [&](const result_t &res) { EXPECT_EQ(3, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::rpush("list1", {"value4", "value5", "value6"}),
        [&](const result_t &res) { EXPECT_EQ(6, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::lpop("list1"),
        [&](const result_t &res) { EXPECT_EQ("value3", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::rpop("list1"),
        [&](const result_t &res) { EXPECT_EQ("value6", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::llen("list1"),
        [&](const result_t &res) { EXPECT_EQ(4, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::lrange("list1", 0, -1),
        [&](const result_t &res) {
            auto values = boost::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 4);
            EXPECT_EQ("value2", boost::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("value1", boost::get<redis_async::string_t>(values.elements[1]));
            EXPECT_EQ("value4", boost::get<redis_async::string_t>(values.elements[2]));
            EXPECT_EQ("value5", boost::get<redis_async::string_t>(values.elements[3]));
        },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::lset("list1", 2, "value2"),
        [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::lrem("list1", -7, "value2"),
        [&](const result_t &res) { EXPECT_EQ(2, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::lindex("list1", 0),
        [&](const result_t &res) { EXPECT_EQ("value1", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::lrange("list1", 0, -1),
        [&](const result_t &res) {
            auto values = boost::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 2);
            EXPECT_EQ("value1", boost::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("value5", boost::get<redis_async::string_t>(values.elements[1]));
        },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::lpush("list1", {"value1", "value2", "value3"}),
        [&](const result_t &res) { EXPECT_EQ(5, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::rpush("list1", {"value4", "value5", "value6"}),
        [&](const result_t &res) { EXPECT_EQ(8, boost::get<redis_async::int_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::ltrim("list1", 0, 5),
        [&](const result_t &res) { EXPECT_EQ("OK", boost::get<redis_async::string_t>(res)); },
        [&](const error::rd_error &err) {
            timer.cancel();
            rd_service::stop();
            FAIL() << err.what();
        });

    rd_service::execute(
        "tcp"_rd, cmd::lrange("list1", 0, -1),
        [&](const result_t &res) {
            auto values = boost::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 6);
            EXPECT_EQ("value3", boost::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("value2", boost::get<redis_async::string_t>(values.elements[1]));
            EXPECT_EQ("value1", boost::get<redis_async::string_t>(values.elements[2]));
            EXPECT_EQ("value1", boost::get<redis_async::string_t>(values.elements[3]));
            EXPECT_EQ("value5", boost::get<redis_async::string_t>(values.elements[4]));
            EXPECT_EQ("value4", boost::get<redis_async::string_t>(values.elements[5]));
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
