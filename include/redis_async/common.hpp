//
// Created by niko on 23.05.2021.
//

#ifndef REDIS_ASYNC_COMMON_HPP
#define REDIS_ASYNC_COMMON_HPP

#include <redis_async/error.hpp>
#include <redis_async/rd_types.hpp>

#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>
#include <chrono>
#include <cxxabi.h>
#include <functional>
#include <memory>
#include <string>

namespace redis_async {

    /**
     * Type name demangle function
     * Usage:
     * @code
     * ::std::cout << demangle< ::std::iostream >() << "\n"
     * @endcode
     * @return Demangled type name
     */
    template <typename T>
    ::std::string demangle() {
        int status{0};
        char *ret = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
        ::std::string res{ret};
        if (ret)
            free(ret);
        return res;
    }

    namespace details {
        class basic_connection;
    } // namespace details
    using connection_ptr = std::shared_ptr<details::basic_connection>;
    using optional_size = boost::optional<size_t>;

    using simple_callback = std::function<void()>;
    /** @brief Callback for error handling */
    using error_callback = std::function<void(error::rd_error const &)>;
    /** @brief Callback for query results */
    using query_result_callback = std::function<void(result_t)>;
    /** @brief Callback for a query error */
    using query_error_callback = std::function<void(error::query_error const &)>;

    /**
     * @brief Short unique string to refer a database alias.
     * Signature structure, to pass instead of connection string
     * @see @ref connstring
     * @see
     */
    struct rdalias : std::string {
        using base_type = std::string;

        rdalias() noexcept
            : base_type() {
        }
        explicit rdalias(std::string const &rhs)
            : base_type(rhs) {
        }

        void swap(rdalias &rhs) noexcept {
            base_type::swap(rhs);
        }
        void swap(std::string &rhs) noexcept {
            base_type::swap(rhs);
        }

        rdalias &operator=(std::string const &rhs) {
            rdalias tmp(rhs);
            swap(tmp);
            return *this;
        }
    };
    /**
     * @brief Redis connection options
     */
    struct connection_options {
        rdalias alias;      ///< Alias
        std::string schema; ///< Database connection schema. Currently supported are tcp and socket
        std::string uri;    ///< Connection uri. `host:port` for tcp, `/path/to/file` for socket
        std::string database;                         ///< Database id
        std::string password;                         ///< Database user's password
        bool keep_alive = false;                      ///< keep alive connection
        std::chrono::milliseconds connect_timeout{0}; ///<
        std::chrono::milliseconds socket_timeout{0};  ///<

        /**
         * Parse a connection string
         * @code{.cpp}
         * // Full options for a TCP connection
         * connection_options opts = "aliasname=tcp://user:password@localhost:5432/database"_redis;
         * // Connection via UNIX socket
         * opts = "aliasname=unix:///tmp/.s.REDIS.5432/database"_redis;
         * @endcode
         * @see connstring
         */
        static connection_options parse(std::string const &literal);
    };

} // namespace redis_async

redis_async::rdalias operator"" _rd(const char *, size_t n);

/**
 * User-defined literal for a Redis connection string
 * @code{.cpp}
 * // Full options for a TCP connection
 * connection_options opts = "aliasname=tcp://password@localhost:5432/database"_redis;
 * // Connection via UNIX socket
 * opts = "aliasname=unix:///tmp/.s.REDIS.5432/database"_redis;
 * @endcode
 */
redis_async::connection_options operator"" _redis(const char *, size_t);

#endif // REDIS_ASYNC_COMMON_HPP
