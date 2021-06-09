//
// Created by niko on 09.06.2021.
//

#ifndef REDIS_ASYNC_BASE_CONNECTION_HPP
#define REDIS_ASYNC_BASE_CONNECTION_HPP

#include <redis_async/asio_config.hpp>
#include <redis_async/common.hpp>
#include <redis_async/error.hpp>

#include <boost/noncopyable.hpp>
#include <memory>

#include "events.hpp"

namespace redis_async {
    namespace details {

        class basic_connection;
        using basic_connection_ptr = std::shared_ptr<basic_connection>;

        using connection_event_callback = std::function<void(basic_connection_ptr)>;
        using connection_error_callback =
            std::function<void(basic_connection_ptr, error::connection_error)>;

        struct connection_callbacks {
            connection_event_callback idle;
            connection_event_callback terminated;
            connection_error_callback error;
        };

        class basic_connection : public boost::noncopyable {
        public:
            using io_service_ptr = asio_config::io_service_ptr;

        public:
            static basic_connection_ptr create(io_service_ptr svc, connection_options const &opts,
                                               connection_callbacks const &callbacks);

        public:
            virtual ~basic_connection() = default;

            void connect(connection_options const &opts);
            rdalias const &alias() const;
            void execute(events::execute &&evt);
            void terminate();

        protected:
            basic_connection() = default;

        private:
            virtual void connectImpl(connection_options const &opts) = 0;
            virtual rdalias const &aliasImpl() const = 0;
            virtual void executeImpl(events::execute &&evt) = 0;
            virtual void terminateImpl() = 0;
        };

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_BASE_CONNECTION_HPP
