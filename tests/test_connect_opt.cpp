//
// Created by niko on 23.05.2021.
//

#include <gtest/gtest.h>

#include <redis_async/common.hpp>

TEST(ConnectOptTest, rdalias) {
    auto value = "main"_rd;
    ASSERT_EQ(value, "main");
    value = ""_rd;
    ASSERT_EQ(value, "");
    value = "logs"_rd;
    ASSERT_EQ(value, "logs");
}

TEST(ConnectOptTest, tcp_localhost) {
    auto conn = "main=tcp://password@localhost:6379/1"_redis;
    ASSERT_EQ(conn.alias, "main");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "password");
    ASSERT_EQ(conn.uri, "localhost:6379");
    ASSERT_EQ(conn.database, "1");
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(0));
}

TEST(ConnectOptTest, tcp_ip) {
    auto conn = "main=tcp://password@192.168.0.10:6379/1"_redis;
    ASSERT_EQ(conn.alias, "main");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "password");
    ASSERT_EQ(conn.uri, "192.168.0.10:6379");
    ASSERT_EQ(conn.database, "1");
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(0));
}

TEST(ConnectOptTest, tcp_ip_password) {
    auto conn = "main=tcp://user:password@192.168.0.10:6379/1"_redis;
    ASSERT_EQ(conn.alias, "main");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "user");
    ASSERT_EQ(conn.password, "password");
    ASSERT_EQ(conn.uri, "192.168.0.10:6379");
    ASSERT_EQ(conn.database, "1");
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(0));
}

TEST(ConnectOptTest, without_user) {
    auto conn = "main=tcp://192.168.0.10:6379/1"_redis;
    ASSERT_EQ(conn.alias, "main");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "");
    ASSERT_EQ(conn.uri, "192.168.0.10:6379");
    ASSERT_EQ(conn.database, "1");
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(0));
}

TEST(ConnectOptTest, without_user_db) {
    auto conn = "main=tcp://192.168.0.10:6379"_redis;
    ASSERT_EQ(conn.alias, "main");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "");
    ASSERT_EQ(conn.uri, "192.168.0.10:6379");
    ASSERT_EQ(conn.database, "");
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(0));
}

TEST(ConnectOptTest, without_port) {
    auto conn = "main=tcp://192.168.0.10"_redis;
    ASSERT_EQ(conn.alias, "main");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "");
    ASSERT_EQ(conn.uri, "192.168.0.10");
    ASSERT_EQ(conn.database, "");
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(0));
}

TEST(ConnectOptTest, keep_alive) {
    auto conn = "main=tcp://192.168.0.10:6379?keep_alive=true"_redis;
    ASSERT_EQ(conn.alias, "main");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "");
    ASSERT_EQ(conn.uri, "192.168.0.10:6379");
    ASSERT_EQ(conn.database, "");
    ASSERT_EQ(conn.keep_alive, true);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(0));

    conn = "main=tcp://192.168.0.10:6379?keep_alive=false"_redis;
    ASSERT_EQ(conn.alias, "main");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "");
    ASSERT_EQ(conn.uri, "192.168.0.10:6379");
    ASSERT_EQ(conn.database, "");
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(0));
}

TEST(ConnectOptTest, socket_timeout) {
    auto conn = "main=tcp://192.168.0.10:6379?socket_timeout=732ms"_redis;
    ASSERT_EQ(conn.alias, "main");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "");
    ASSERT_EQ(conn.uri, "192.168.0.10:6379");
    ASSERT_EQ(conn.database, "");
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(732));

    conn = "main=tcp://192.168.0.10:6379?socket_timeout=100s"_redis;
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.socket_timeout, std::chrono::seconds(100));

    conn = "main=tcp://192.168.0.10:6379?socket_timeout=1m"_redis;
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.socket_timeout, std::chrono::minutes(1));
}

TEST(ConnectOptTest, connect_timeout) {
    auto conn = "main=tcp://192.168.0.10:6379?connect_timeout=732ms"_redis;
    ASSERT_EQ(conn.alias, "main");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "");
    ASSERT_EQ(conn.uri, "192.168.0.10:6379");
    ASSERT_EQ(conn.database, "");
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(732));
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds());

    conn = "main=tcp://192.168.0.10:6379?connect_timeout=100s"_redis;
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.connect_timeout, std::chrono::seconds(100));

    conn = "main=tcp://192.168.0.10:6379?connect_timeout=1m"_redis;
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(0));
    ASSERT_EQ(conn.connect_timeout, std::chrono::minutes(1));
}

TEST(ConnectOptTest, combain_opts) {
    auto conn = "main=tcp://192.168.0.10:6379?keep_alive=true&connect_timeout=732ms"_redis;
    ASSERT_EQ(conn.alias, "main");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "");
    ASSERT_EQ(conn.uri, "192.168.0.10:6379");
    ASSERT_EQ(conn.database, "");
    ASSERT_EQ(conn.keep_alive, true);
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(732));
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds());

    conn = "main=tcp://192.168.0.10:6379?keep_alive=true&socket_timeout=732ms"_redis;
    ASSERT_EQ(conn.keep_alive, true);
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(732));
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(0));

    conn = "main=tcp://192.168.0.10:6379?socket_timeout=732s&connect_timeout=1m"_redis;
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.socket_timeout, std::chrono::seconds(732));
    ASSERT_EQ(conn.connect_timeout, std::chrono::minutes(1));

    conn = "main=tcp://192.168.0.10:6379?keep_alive=true&socket_timeout=732s&connect_timeout=1m"_redis;
    ASSERT_EQ(conn.keep_alive, true);
    ASSERT_EQ(conn.socket_timeout, std::chrono::seconds(732));
    ASSERT_EQ(conn.connect_timeout, std::chrono::minutes(1));

    conn = "logs=tcp://127.0.0.1?socket_timeout=50ms&connect_timeout=1s"_redis;
    ASSERT_EQ(conn.alias, "logs");
    ASSERT_EQ(conn.schema, "tcp");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "");
    ASSERT_EQ(conn.uri, "127.0.0.1");
    ASSERT_EQ(conn.database, "");
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(50));
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(1000));
}

TEST(ConnectOptTest, uds) {
    auto conn = "logs=unix:///path/to/socket?socket_timeout=50ms&connect_timeout=1s"_redis;
    ASSERT_EQ(conn.alias, "logs");
    ASSERT_EQ(conn.schema, "unix");
    ASSERT_EQ(conn.user, "");
    ASSERT_EQ(conn.password, "");
    ASSERT_EQ(conn.uri, "/path/to/socket");
    ASSERT_EQ(conn.database, "");
    ASSERT_EQ(conn.keep_alive, false);
    ASSERT_EQ(conn.socket_timeout, std::chrono::milliseconds(50));
    ASSERT_EQ(conn.connect_timeout, std::chrono::milliseconds(1000));

}
TEST(ConnectOptTest, wrong_alias) {
    ASSERT_THROW(auto conn = "tcp://127.0.0.1"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "unix://127.0.0.1dsas"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "=tcp://127.0.0.1"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "="_redis, std::runtime_error);
    ASSERT_THROW(auto conn = ""_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "hello, world"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main="_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp://"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=://asdds"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=:/"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=://"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp://localhost:7432?keep"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp://localhost:7432?keep_alive"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp://localhost:7432?keepalive"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp://localhost:7432?keep_alive=1"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp://localhost:7432?keep_alive=0"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp://localhost:7432?keep_alive="_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp://localhost:7432?keepalive=as"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp://localhost:7432?timeout=10s"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp://localhost:7432?socket_timeout=a10s"_redis, std::runtime_error);
    ASSERT_THROW(auto conn = "main=tcp://localhost:7432?socket_timeout=10hour"_redis, std::runtime_error);
}

//TEST(ConnectOptTest, uds) {
//    auto conn = "main=unix://192.168.0.10:6379"_redis;
//    ASSERT_EQ(conn.alias, "main");
//    ASSERT_EQ(conn.schema, "tcp");
//    ASSERT_EQ(conn.user, "user");
//    ASSERT_EQ(conn.password, "password");
//    ASSERT_EQ(conn.uri, "192.168.0.10:6379");
//    ASSERT_EQ(conn.database, "1");
//}
