//
// Created by niko on 23.05.2021.
//

#include "details/connection/base_connection.hpp"
#include "details/redis_impl.hpp"
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
    } // namespace

    void rd_service::initialize(size_t pool_size) {
        lock_type lock(db_service_lock());

        auto &pimpl = impl_ptr();
        if (!pimpl) {
            pimpl.reset(new details::redis_impl(pool_size));
        } else {
            pimpl->set_defaults(pool_size);
        }
    }

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

    void rd_service::execute(rdalias const &alias, const std::string &expression,
                             query_result_callback const &result, error_callback const &error) {
        // TODO Wrap callbacks in strands
        impl()->get_connection(alias, expression, result, error);
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