//
// Created by niko on 02.07.2021.
//
#include <redis_async/details/connection/concrete_connection.hpp>
#include <redis_async/details/connection/transport.hpp>

#include <gtest/gtest.h>

namespace asio_config = redis_async::asio_config;

struct dummy_transport {
    using io_service_ptr = asio_config::io_service_ptr;
    using connect_callback = std::function<void(const asio_config::error_code &)>;
    using connection_options = redis_async::connection_options;

    dummy_transport(io_service_ptr) {
    }

    void connect_async(const connection_options &, const connect_callback &cb) {
        asio_config::error_code ec;
        cb(ec);
    }

    bool connected() const {
        return true;
    }

    void close() {
    }

    template <typename BufferType, typename Handler>
    void async_read(const BufferType &, Handler) {
    }

    template <typename BufferType, typename Handler>
    void async_write(const BufferType &, Handler) {
    }
};

using fsm = redis_async::details::concrete_connection<dummy_transport>;
using fsm_ptr = std::shared_ptr<fsm>;

enum class States { unplugged, connecting, authn, idle, query, terminated, StatesCount };

TEST(TestFSM, NormalFlow) {
    using redis_async::details::events::complete;
    using redis_async::details::events::execute;
    using redis_async::details::events::recv;
    using redis_async::details::events::terminate;
    using redis_async::error::query_error;

    asio_config::io_service_ptr svc(new asio_config::io_service);

    fsm_ptr c(new fsm(svc, {}));
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::unplugged));

    // unplug -> conn_opts -> connecting -> complete -> auth
    /// @todo нужно connect_async переделать в dummy_transport
    c->process_event("main=tcp://password@localhost:6379/1"_redis);
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::authn));

    // auth -> complete -> idle
    c->process_event(complete{});
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::idle));

    // idle -> execute -> query
    c->process_event(execute{});
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::query));

    // query -> query_error -> idle
    c->process_event(query_error(""));
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::idle));

    // again got to query
    // store terminate event
    // stay in query
    c->process_event(execute{});
    c->process_event(terminate{});
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::query));

    // query -> recv -> idle -> terminate -> terminated
    c->process_event(recv{});
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::terminated));
}

TEST(TestFSM, AuthnFlow) {
    using redis_async::details::events::complete;
    using redis_async::details::events::execute;
    using redis_async::details::events::recv;
    using redis_async::details::events::terminate;
    using redis_async::error::query_error;

    asio_config::io_service_ptr svc(new asio_config::io_service);

    fsm_ptr c(new fsm(svc, {}));
    c->process_event("main=tcp://password@localhost:6379/1"_redis);
    c->process_event(recv{});
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::idle));

    // idle -> execute -> query
    c->process_event(terminate{});
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::terminated));
}

TEST(TestFSM, TerminateFlow) {
    using redis_async::details::events::complete;
    using redis_async::details::events::execute;
    using redis_async::details::events::recv;
    using redis_async::details::events::terminate;
    using redis_async::error::connection_error;

    asio_config::io_service_ptr svc(new asio_config::io_service);

    // unplug -> terminated
    fsm_ptr c(new fsm(svc, {}));
    c->process_event(terminate());
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::terminated));

    // unplug -> connecting -> auth -> terminated
    c.reset(new fsm(svc, {}));
    c->process_event("main=tcp://password@localhost:6379/1"_redis);
    c->process_event(terminate());
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::authn));
    c->process_event(recv{});
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::terminated));

    // unplug -> connecting -> auth -> idle -> terminated
    c.reset(new fsm(svc, {}));
    c->process_event("main=tcp://password@localhost:6379/1"_redis);
    c->process_event(recv{});
    c->process_event(terminate());
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::terminated));

    // unplug -> connecting -> auth -> idle -> query -> terminated
    c.reset(new fsm(svc, {}));
    c->process_event("main=tcp://password@localhost:6379/1"_redis);
    c->process_event(recv{});
    c->process_event(execute{});
    c->process_event(terminate());
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::query));
    c->process_event(recv{});
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::terminated));

    // unplug -> connecting -> auth -> idle -> query -> idle -> terminated
    c.reset(new fsm(svc, {}));
    c->process_event("main=tcp://password@localhost:6379/1"_redis);
    c->process_event(recv{});
    c->process_event(execute{});
    c->process_event(recv{});
    c->process_event(terminate());
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::terminated));
}

TEST(TestFSM, ConnectionErrorFlow) {
    using redis_async::details::events::complete;
    using redis_async::details::events::execute;
    using redis_async::details::events::recv;
    using redis_async::details::events::terminate;
    using redis_async::error::connection_error;

    asio_config::io_service_ptr svc(new asio_config::io_service);

    // unplug -> connecting -> auth -> terminated
    fsm_ptr c(new fsm(svc, {}));
    c->process_event("main=tcp://password@localhost:6379/1"_redis);
    c->process_event(connection_error(""));
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::terminated));

    // unplug -> connecting -> auth -> idle -> query -> terminated
    c.reset(new fsm(svc, {}));
    c->process_event("main=tcp://password@localhost:6379/1"_redis);
    c->process_event(recv{});
    c->process_event(execute{});
    c->process_event(connection_error(""));
    for (int i = 0; i < fsm::nr_regions::value; ++i)
        ASSERT_EQ(c->current_state()[i], static_cast<int>(States::terminated));
}

TEST(TestFSM, InvalidEvent) {
    using redis_async::details::events::complete;
    using redis_async::details::events::execute;
    using redis_async::details::events::recv;
    using redis_async::details::events::terminate;
    using redis_async::error::connection_error;

    asio_config::io_service_ptr svc(new asio_config::io_service);

    // unplug -> connecting -> auth -> terminated
    fsm_ptr c(new fsm(svc, {}));
    c->process_event("main=tcp://password@localhost:6379/1"_redis);
    ASSERT_THROW(c->process_event("main=tcp://password@localhost:6379/1"_redis), std::exception);
}
