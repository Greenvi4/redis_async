//
// Created by niko on 23.05.2021.
//
#include <redis_async/redis_async.hpp>

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(resi_async, test) {
    using redis_async::Redis;

    Redis r;
}
