//
// Created by niko on 23.05.2021.
//

#ifndef REDIS_ASYNC_COMMON_HPP
#define REDIS_ASYNC_COMMON_HPP

#include <chrono>
#include <string>

namespace redis_async {

    /**
     * @brief Short unique string to refer a database.
     * Signature structure, to pass instead of connection string
     * @see @ref connstring
     * @see tip::db::pg::db_service
     */
    struct rdalias : std::string {
        using base_type = std::string;

        rdalias()
            : base_type() {
        }
        explicit rdalias(std::string const &rhs)
            : base_type(rhs) {
        }

        void swap(rdalias &rhs) /* no_throw */
        {
            base_type::swap(rhs);
        }
        void swap(std::string &rhs) /* no_throw */
        {
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
        std::string database; ///< Database id
        std::string user;     ///< Database user name
        std::string password; ///< Database user's password
        bool keep_alive = false;
        std::chrono::milliseconds connect_timeout{0};
        std::chrono::milliseconds socket_timeout{0};

        /**
         * Parse a connection string
         * @code{.cpp}
         * // Full options for a TCP connection
         * connection_options opts = "aliasname=tcp://user:password@localhost:5432[database]"_pg;
         * // SSL connection over TCP
         * opts = "ssl://localhost:5432[database]"_pg;
         * // Connection via UNIX socket
         * opts = "socket:///tmp/.s.PGSQL.5432[database]"_pg;
         * @endcode
         * @see connstring
         */
        static connection_options parse(std::string const &literal);
    };

} // namespace redis_async

redis_async::rdalias operator"" _rd(const char *, size_t n);

/**
 * User-defined literal for a PostgreSQL connection string
 * @code{.cpp}
 * // Full options for a TCP connection
 * connection_options opts = "aliasname=tcp://user:password@localhost:5432[database]"_pg;
 * // SSL connection over TCP
 * opts = "ssl://localhost:5432[database]"_pg;
 * // Connection via UNIX socket
 * opts = "socket:///tmp/.s.PGSQL.5432[database]"_pg;
 * @endcode
 */
redis_async::connection_options operator"" _redis(const char *, size_t);

#endif // REDIS_ASYNC_COMMON_HPP
