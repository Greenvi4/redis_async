//
// Created by niko on 10.06.2021.
//

#include <gtest/gtest.h>

#include <redis_async/redis_async.hpp>

TEST(ConnectionTest, def) {
    using redis_async::rd_service;
    rd_service::add_connection("main=tcp://localhost:6379"_redis);
}
