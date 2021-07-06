#ifndef PROCESS_CONFIG_HPP
#define PROCESS_CONFIG_HPP

#include "exception.hpp"

namespace process {
    namespace details {

        inline std::error_code get_last_error() noexcept {
            return std::error_code(errno, std::system_category());
        }

        inline void throw_error(std::error_code ec) {
            if (ec)
                throw process_error(ec);
        }

        inline void throw_error(std::error_code ec, const char *msg) {
            if (ec)
                throw process_error(ec, msg);
        }

    } // namespace details
} // namespace process

#endif // PROCESS_CONFIG_HPP
