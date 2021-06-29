//
// Created by niko on 11.06.2021.
//

#ifndef REDIS_ASYNC_TEST_SERVER_HPP
#define REDIS_ASYNC_TEST_SERVER_HPP

#include <boost/algorithm/string/join.hpp>
#include <log4cxx/logger.h>
#include <memory>

#include "process/child.hpp"

namespace test_server {
    struct TestServer {
        using child_t = std::unique_ptr<process::child>;
        child_t child;
        log4cxx::LoggerPtr logger;

        TestServer(std::initializer_list<std::string> &&args)
            : logger(log4cxx::Logger::getLogger("redis_async.test.server")) {
            std::string str = boost::algorithm::join(args, " ");
            LOG4CXX_INFO(logger, "going to fork to start: " << str);
            auto process = new process::child(str);
            child.reset(process);
        }
        ~TestServer() {
            LOG4CXX_INFO(logger, "terminating child " << child->id());
        }
    };

    using result_t = std::unique_ptr<TestServer>;

    inline result_t make_server(std::initializer_list<std::string> &&args) {
        return std::make_unique<TestServer>(std::move(args));
    }
} // namespace test_server

#endif // REDIS_ASYNC_TEST_SERVER_HPP
