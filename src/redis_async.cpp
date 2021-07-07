//
// Created by niko on 23.05.2021.
//

#include <redis_async/details/connection/base_connection.hpp>
#include <redis_async/details/redis_impl.hpp>
#include <redis_async/redis_async.hpp>

#include <mutex>
#include <utility>

namespace redis_async {

    typedef std::recursive_mutex mutex_type;
    typedef std::lock_guard<mutex_type> lock_type;

    namespace {
        mutex_type &db_service_lock() {
            static mutex_type _mtx;
            return _mtx;
        }

        std::string set_update_type(UpdateType udp) {
            switch (udp) {
            case UpdateType::exist:
                return "XX";
            case UpdateType::not_exist:
                return "NX";
            default:
                break;
            }
            throw error::client_error("Invalid update type " +
                                      std::to_string(static_cast<int>(udp)));
        }

    } // namespace

    void rd_service::add_connection(const std::string &connection_string, optional_size pool_size) {
        impl()->add_connection(connection_string, std::move(pool_size));
    }

    void rd_service::add_connection(const connection_options &co, optional_size pool_size) {
        impl()->add_connection(co, std::move(pool_size));
    }

    void rd_service::run() {
        impl()->run();
    }

    void rd_service::stop() {
        lock_type lock(db_service_lock());
        LOG4CXX_INFO(details::logger, "Stop db service");

        auto &pimpl = impl_ptr();
        if (pimpl) {
            pimpl->stop();
        }
        pimpl.reset();
    }

    asio_config::io_service_ptr rd_service::io_service() {
        return impl()->io_service();
    }

    void rd_service::ping(const rdalias &alias, const query_result_callback &result,
                          const error_callback &error) {
        // TODO Wrap callbacks in strands
        impl()->get_connection(alias, "PING", result, error);
    }

    void rd_service::ping(const rdalias &alias, boost::string_ref msg,
                          const query_result_callback &result, const error_callback &error) {
        // TODO Wrap callbacks in strands
        using details::single_command_t;
        impl()->get_connection(alias, single_command_t{"PING", msg}, result, error);
    }

    void rd_service::echo(const rdalias &alias, boost::string_ref msg,
                          const query_result_callback &result, const error_callback &error) {
        // TODO Wrap callbacks in strands
        using details::single_command_t;
        impl()->get_connection(alias, single_command_t{"ECHO", msg}, result, error);
    }

    void rd_service::set(const rdalias &alias, boost::string_ref key, boost::string_ref value,
                         const query_result_callback &result, const error_callback &error) {
        // TODO Wrap callbacks in strands
        using details::single_command_t;
        impl()->get_connection(alias, single_command_t{"SET", key, value}, result, error);
    }

    void rd_service::set(const rdalias &alias, boost::string_ref key, boost::string_ref value,
                         UpdateType udp, const query_result_callback &result,
                         const error_callback &error) {
        // TODO Wrap callbacks in strands
        using details::single_command_t;
        impl()->get_connection(alias, single_command_t{"SET", key, value, set_update_type(udp)},
                               result, error);
    }

    void rd_service::set(const rdalias &alias, boost::string_ref key, boost::string_ref value,
                         const std::chrono::milliseconds &ttl, const query_result_callback &result,
                         const error_callback &error) {
        // TODO Wrap callbacks in strands
        using details::single_command_t;
        impl()->get_connection(
            alias, single_command_t{"SET", key, value, "PX", std::to_string(ttl.count())}, result,
            error);
    }

    void rd_service::set(const rdalias &alias, boost::string_ref key, boost::string_ref value,
                         UpdateType udp, const std::chrono::milliseconds &ttl,
                         const query_result_callback &result, const error_callback &error) {
        // TODO Wrap callbacks in strands
        using details::single_command_t;
        impl()->get_connection(alias,
                               single_command_t{"SET", key, value, "PX",
                                                std::to_string(ttl.count()), set_update_type(udp)},
                               result, error);
    }

    void rd_service::get(const rdalias &alias, boost::string_ref key,
                         const query_result_callback &result, const error_callback &error) {
        // TODO Wrap callbacks in strands
        using details::single_command_t;
        impl()->get_connection(alias, single_command_t{"GET", key}, result, error);
    }

    rd_service::pimpl &rd_service::impl_ptr() {
        static pimpl p;
        return p;
    }

    rd_service::pimpl rd_service::impl(size_t pool_size) {
        lock_type lock(db_service_lock());

        auto &pimpl = impl_ptr();
        if (!pimpl) {
            pimpl.reset(new details::redis_impl(pool_size));
        }
        return pimpl;
    }

} // namespace redis_async