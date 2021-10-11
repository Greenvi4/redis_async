//
// Created by niko on 29.06.2021.
//

#include "redis_instance.hpp"

#include <boost/lexical_cast.hpp>
#include <fstream>
#include <gtest/gtest.h>

namespace rt = redis_async::test::instance;

auto on_time_expiry = [](boost::system::error_code ec) {
    if (ec)
        return;
    using redis_async::rd_service;
    rd_service::stop();
    FAIL() << "Test timer expired";
};

auto on_rd_error = [](std::unique_ptr<rt::Client> &inst, const redis_async::error::rd_error &err) {
    inst.reset();
    FAIL() << err.what();
};

TEST(CommandsTest, ping) {
    using redis_async::rd_service;
    using redis_async::result_t;
    namespace cmd = redis_async::cmd;

    auto inst = std::make_unique<rt::Client>();
    inst->add_connection("tcp", 1);
    inst->add_deadline_timer(boost::posix_time::seconds(5), on_time_expiry);
    auto error_handler = std::bind(on_rd_error, boost::ref(inst), std::placeholders::_1);

    rd_service::execute(
        "tcp"_rd, cmd::ping(),
        [](const result_t &res) { EXPECT_EQ("PONG", std::get<redis_async::string_t>(res)); },
        error_handler);

    const std::string msg = "Hello, World!";
    rd_service::execute(
        "tcp"_rd, cmd::ping(msg),
        [msg](const result_t &res) { EXPECT_EQ(msg, std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::ping(""),
        [&](const result_t &res) {
            EXPECT_EQ("", std::get<redis_async::string_t>(res));
            inst.reset();
        },
        error_handler);

    inst->run();
}

TEST(CommandsTest, echo) {
    using redis_async::rd_service;
    using redis_async::result_t;
    namespace cmd = redis_async::cmd;

    auto inst = std::make_unique<rt::Client>();
    inst->add_connection("tcp", 1);
    inst->add_deadline_timer(boost::posix_time::seconds(5), on_time_expiry);
    auto error_handler = std::bind(on_rd_error, boost::ref(inst), std::placeholders::_1);

    const std::string msg = "Hello, World!";

    rd_service::execute(
        "tcp"_rd, cmd::echo(msg),
        [&](const result_t &res) { EXPECT_EQ(msg, std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::echo(""),
        [&](const result_t &res) {
            EXPECT_EQ("", std::get<redis_async::string_t>(res));
            inst.reset();
        },
        error_handler);

    rd_service::run();
}

TEST(CommandsTest, set_get) {
    using redis_async::rd_service;
    using redis_async::result_t;
    namespace error = redis_async::error;
    namespace cmd = redis_async::cmd;

    auto inst = std::make_unique<rt::Client>();
    inst->add_connection("tcp", 1);
    inst->add_deadline_timer(boost::posix_time::seconds(5), on_time_expiry);
    auto error_handler = std::bind(on_rd_error, boost::ref(inst), std::placeholders::_1);

    const std::string key = "some_key";
    const std::string value = "some_value";

    rd_service::execute(
        "tcp"_rd, cmd::set(key, value),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::set(key, value, redis_async::UpdateType::exist),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::set(key, value, redis_async::UpdateType::not_exist),
        [&](const result_t &res) { EXPECT_NO_THROW(std::get<redis_async::nil_t>(res)); },
        error_handler);

    ASSERT_THROW(
        rd_service::execute(
            "tcp"_rd, cmd::set(key, value, static_cast<redis_async::UpdateType>(100)),
            [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
            error_handler),
        error::client_error);

    using std::chrono::milliseconds;

    rd_service::execute(
        "tcp"_rd, cmd::set("expire_key_1", value, milliseconds(120)),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::set("expire_key_2", value, milliseconds(0)),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::set("expire_key_2", value, milliseconds(-100)),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::get(key),
        [&](const result_t &res) { EXPECT_EQ(value, std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::get("another_key"),
        [&](const result_t &res) {
            EXPECT_NO_THROW(std::get<redis_async::nil_t>(res));
            inst.reset();
        },
        error_handler);

    rd_service::run();
}

TEST(CommandsTest, keys) {
    using redis_async::rd_service;
    using redis_async::result_t;
    namespace cmd = redis_async::cmd;

    auto inst = std::make_unique<rt::Client>();
    inst->add_connection("tcp", 1);
    inst->add_deadline_timer(boost::posix_time::seconds(5), on_time_expiry);
    auto error_handler = std::bind(on_rd_error, boost::ref(inst), std::placeholders::_1);

    rd_service::execute(
        "tcp"_rd, cmd::mset({{"key1", "value1"}, {"key2", "value2"}, {"key3", "value3"}}),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::del({"key3", "nokey"}),
        [&](const result_t &res) { EXPECT_EQ(1, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::exists({"key2", "nokey"}),
        [&](const result_t &res) { EXPECT_EQ(1, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::exists({"key1", "key2"}),
        [&](const result_t &res) { EXPECT_EQ(2, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::expire("key1", std::chrono::seconds(10)),
        [&](const result_t &res) { EXPECT_EQ(1, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::ttl("key1"),
        [&](const result_t &res) { EXPECT_EQ(10, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::pexpire("key2", std::chrono::seconds(1)),
        [&](const result_t &res) { EXPECT_EQ(1, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::pttl("key2"),
        [&](const result_t &res) {
            EXPECT_GE(1000, std::get<redis_async::int_t>(res));
            EXPECT_LE(990, std::get<redis_async::int_t>(res));
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::rename("key1", "another_key1"),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::keys("*key*"),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            std::sort(std::begin(values.elements), std::end(values.elements),
                      [](const auto &l, const auto &r) {
                          return std::get<redis_async::string_t>(l) <
                                 std::get<redis_async::string_t>(r);
                      });
            EXPECT_EQ(values.elements.size(), 2);
            EXPECT_EQ("another_key1", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("key2", std::get<redis_async::string_t>(values.elements[1]));
            inst.reset();
        },
        error_handler);

    rd_service::run();
}

TEST(CommandsTest, mset_mget) {
    using redis_async::rd_service;
    using redis_async::result_t;
    namespace error = redis_async::error;
    namespace cmd = redis_async::cmd;

    auto inst = std::make_unique<rt::Client>();
    inst->add_connection("tcp", 1);
    inst->add_deadline_timer(boost::posix_time::seconds(5), on_time_expiry);
    auto error_handler = std::bind(on_rd_error, boost::ref(inst), std::placeholders::_1);

    rd_service::execute(
        "tcp"_rd, cmd::mset({{"key1", "value1"}, {"key2", "value2"}, {"key3", "value3"}}),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::mset({{"key1", ""}, {"key2", ""}, {"key3", ""}}),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::mget({"key1", "key2", "key3"}),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 3);
            EXPECT_EQ("", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("", std::get<redis_async::string_t>(values.elements[1]));
            EXPECT_EQ("", std::get<redis_async::string_t>(values.elements[2]));
            inst.reset();
        },
        error_handler);

    ASSERT_THROW(
        rd_service::execute(
            "tcp"_rd, cmd::mset({}),
            [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
            error_handler),
        error::client_error);

    ASSERT_THROW(rd_service::execute(
                     "tcp"_rd, cmd::mget({}), [&](const result_t &) {}, error_handler),
                 error::client_error);

    rd_service::run();
}

TEST(CommandsTest, hash) {
    using redis_async::rd_service;
    using redis_async::result_t;
    namespace cmd = redis_async::cmd;

    auto inst = std::make_unique<rt::Client>();
    inst->add_connection("tcp", 1);
    inst->add_deadline_timer(boost::posix_time::seconds(5), on_time_expiry);
    auto error_handler = std::bind(on_rd_error, boost::ref(inst), std::placeholders::_1);

    rd_service::execute(
        "tcp"_rd, cmd::hset("hash", {{"key1", "value1"}, {"key2", "value2"}, {"key3", "value3"}}),
        [&](const result_t &res) { EXPECT_EQ(3, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::hget("hash", "key1"),
        [&](const result_t &res) { EXPECT_EQ("value1", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::hget("hash", "key2"),
        [&](const result_t &res) { EXPECT_EQ("value2", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::hdel("hash", {"key3", "does_not_exists"}),
        [&](const result_t &res) { EXPECT_EQ(1, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::hget("hash", "key3"),
        [&](const result_t &res) { EXPECT_NO_THROW(std::get<redis_async::nil_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::hkeys("hash"),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 2);
            EXPECT_EQ("key1", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("key2", std::get<redis_async::string_t>(values.elements[1]));
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd,
        cmd::hmset(
            "hash",
            {{"key1", "another_value1"}, {"key2", "another_value2"}, {"key3", "another_value3"}}),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::hmget("hash", {"key1", "key2", "key3", "doe_not_exists"}),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 4);
            EXPECT_EQ("another_value1", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("another_value2", std::get<redis_async::string_t>(values.elements[1]));
            EXPECT_EQ("another_value3", std::get<redis_async::string_t>(values.elements[2]));
            EXPECT_NO_THROW(std::get<redis_async::nil_t>(values.elements[3]));
            inst.reset();
        },
        error_handler);

    rd_service::run();
}

TEST(CommandsTest, lists) {
    using redis_async::rd_service;
    using redis_async::result_t;
    namespace cmd = redis_async::cmd;

    auto inst = std::make_unique<rt::Client>();
    inst->add_connection("tcp", 1);
    inst->add_deadline_timer(boost::posix_time::seconds(5), on_time_expiry);
    auto error_handler = std::bind(on_rd_error, boost::ref(inst), std::placeholders::_1);

    rd_service::execute(
        "tcp"_rd, cmd::lpush("list1", {"value1", "value2", "value3"}),
        [&](const result_t &res) { EXPECT_EQ(3, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::rpush("list1", {"value4", "value5", "value6"}),
        [&](const result_t &res) { EXPECT_EQ(6, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::lpop("list1"),
        [&](const result_t &res) { EXPECT_EQ("value3", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::rpop("list1"),
        [&](const result_t &res) { EXPECT_EQ("value6", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::llen("list1"),
        [&](const result_t &res) { EXPECT_EQ(4, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::lrange("list1", 0, -1),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 4);
            EXPECT_EQ("value2", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("value1", std::get<redis_async::string_t>(values.elements[1]));
            EXPECT_EQ("value4", std::get<redis_async::string_t>(values.elements[2]));
            EXPECT_EQ("value5", std::get<redis_async::string_t>(values.elements[3]));
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::lset("list1", 2, "value2"),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::lrem("list1", -7, "value2"),
        [&](const result_t &res) { EXPECT_EQ(2, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::lindex("list1", 0),
        [&](const result_t &res) { EXPECT_EQ("value1", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::lrange("list1", 0, -1),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 2);
            EXPECT_EQ("value1", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("value5", std::get<redis_async::string_t>(values.elements[1]));
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::lpush("list1", {"value1", "value2", "value3"}),
        [&](const result_t &res) { EXPECT_EQ(5, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::rpush("list1", {"value4", "value5", "value6"}),
        [&](const result_t &res) { EXPECT_EQ(8, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::ltrim("list1", 0, 5),
        [&](const result_t &res) { EXPECT_EQ("OK", std::get<redis_async::string_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::lrange("list1", 0, -1),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 6);
            EXPECT_EQ("value3", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("value2", std::get<redis_async::string_t>(values.elements[1]));
            EXPECT_EQ("value1", std::get<redis_async::string_t>(values.elements[2]));
            EXPECT_EQ("value1", std::get<redis_async::string_t>(values.elements[3]));
            EXPECT_EQ("value5", std::get<redis_async::string_t>(values.elements[4]));
            EXPECT_EQ("value4", std::get<redis_async::string_t>(values.elements[5]));
            inst.reset();
        },
        error_handler);

    rd_service::run();
}

TEST(CommandsTest, sets) {
    using redis_async::rd_service;
    using redis_async::result_t;
    namespace cmd = redis_async::cmd;

    auto inst = std::make_unique<rt::Client>();
    inst->add_connection("tcp", 1);
    inst->add_deadline_timer(boost::posix_time::seconds(5), on_time_expiry);
    auto error_handler = std::bind(on_rd_error, boost::ref(inst), std::placeholders::_1);

    rd_service::execute(
        "tcp"_rd, cmd::sadd("set1", {"a", "b", "c", "d"}),
        [&](const result_t &res) { EXPECT_EQ(4, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::sadd("set2", {"c"}),
        [&](const result_t &res) { EXPECT_EQ(1, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::sadd("set3", {"a", "c", "e"}),
        [&](const result_t &res) { EXPECT_EQ(3, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::scard("set1"),
        [&](const result_t &res) { EXPECT_EQ(4, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::scard("not_exists"),
        [&](const result_t &res) { EXPECT_EQ(0, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::smembers("set3"),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 3);
            std::sort(std::begin(values.elements), std::end(values.elements),
                      [](const auto &l, const auto &r) {
                          return std::get<redis_async::string_t>(l) <
                                 std::get<redis_async::string_t>(r);
                      });
            EXPECT_EQ("a", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("c", std::get<redis_async::string_t>(values.elements[1]));
            EXPECT_EQ("e", std::get<redis_async::string_t>(values.elements[2]));
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::sdiff({"set1", "set2", "set3"}),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            std::sort(std::begin(values.elements), std::end(values.elements),
                      [](const auto &l, const auto &r) {
                          return std::get<redis_async::string_t>(l) <
                                 std::get<redis_async::string_t>(r);
                      });
            EXPECT_EQ(values.elements.size(), 2);
            EXPECT_EQ("b", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("d", std::get<redis_async::string_t>(values.elements[1]));
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::sdiffstore("diff_sets", {"set1", "set2", "set3"}),
        [&](const result_t &res) { EXPECT_EQ(2, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::smembers("diff_sets"),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 2);
            std::sort(std::begin(values.elements), std::end(values.elements),
                      [](const auto &l, const auto &r) {
                          return std::get<redis_async::string_t>(l) <
                                 std::get<redis_async::string_t>(r);
                      });
            EXPECT_EQ("b", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("d", std::get<redis_async::string_t>(values.elements[1]));
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::sinter({"set1", "set2", "set3"}),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 1);
            EXPECT_EQ("c", std::get<redis_async::string_t>(values.elements[0]));
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::sinterstore("inter_sets", {"set1", "set2", "set3"}),
        [&](const result_t &res) { EXPECT_EQ(1, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::smembers("inter_sets"),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 1);
            EXPECT_EQ("c", std::get<redis_async::string_t>(values.elements[0]));
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::sunion({"set1", "set2", "set3"}),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            std::sort(std::begin(values.elements), std::end(values.elements),
                      [](const auto &l, const auto &r) {
                          return std::get<redis_async::string_t>(l) <
                                 std::get<redis_async::string_t>(r);
                      });
            EXPECT_EQ(values.elements.size(), 5);
            EXPECT_EQ("a", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("b", std::get<redis_async::string_t>(values.elements[1]));
            EXPECT_EQ("c", std::get<redis_async::string_t>(values.elements[2]));
            EXPECT_EQ("d", std::get<redis_async::string_t>(values.elements[3]));
            EXPECT_EQ("e", std::get<redis_async::string_t>(values.elements[4]));
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::sunionstore("union_sets", {"set1", "set2", "set3"}),
        [&](const result_t &res) { EXPECT_EQ(5, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::smembers("union_sets"),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            std::sort(std::begin(values.elements), std::end(values.elements),
                      [](const auto &l, const auto &r) {
                          return std::get<redis_async::string_t>(l) <
                                 std::get<redis_async::string_t>(r);
                      });
            EXPECT_EQ(values.elements.size(), 5);
            EXPECT_EQ("a", std::get<redis_async::string_t>(values.elements[0]));
            EXPECT_EQ("b", std::get<redis_async::string_t>(values.elements[1]));
            EXPECT_EQ("c", std::get<redis_async::string_t>(values.elements[2]));
            EXPECT_EQ("d", std::get<redis_async::string_t>(values.elements[3]));
            EXPECT_EQ("e", std::get<redis_async::string_t>(values.elements[4]));
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::spop("set1"),
        [&](const result_t &res) {
            auto value = std::get<redis_async::string_t>(res);
            EXPECT_TRUE(value == "a" || value == "b" || value == "c" || value == "d");
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::spop("set1", 2),
        [&](const result_t &res) {
            auto values = std::get<redis_async::array_holder_t>(res);
            EXPECT_EQ(values.elements.size(), 2);
            for (const auto &v : values.elements) {
                auto value = std::get<redis_async::string_t>(v);
                EXPECT_TRUE(value == "a" || value == "b" || value == "c" || value == "d");
            }
        },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::scard("set1"),
        [&](const result_t &res) { EXPECT_EQ(1, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::srem("set3", {"a"}),
        [&](const result_t &res) { EXPECT_EQ(1, std::get<redis_async::int_t>(res)); },
        error_handler);

    rd_service::execute(
        "tcp"_rd, cmd::srem("set3", {"a", "c", "e", "d"}),
        [&](const result_t &res) {
            EXPECT_EQ(2, std::get<redis_async::int_t>(res));
            inst.reset();
        },
        error_handler);

    rd_service::run();
}

TEST(CommandsTest, hadnle_exceptions) {
    using redis_async::rd_service;
    using redis_async::result_t;
    namespace cmd = redis_async::cmd;

    auto inst = std::make_unique<rt::Client>();
    inst->add_connection("tcp", 1);
    inst->add_deadline_timer(boost::posix_time::seconds(5), on_time_expiry);

    rd_service::execute(
        "tcp"_rd, cmd::get("some_wrong_key"),
        [](const auto &res) { std::get<redis_async::string_t>(res); },
        [&](const auto &) { inst.reset(); });

    rd_service::run();
}
