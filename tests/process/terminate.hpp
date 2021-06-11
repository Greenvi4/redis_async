#ifndef PROCESS_TERMINATE_HPP
#define PROCESS_TERMINATE_HPP

#include "child_handle.hpp"
#include "config.hpp"

#include <signal.h>
#include <sys/wait.h>

namespace process {
    namespace details {

        inline void terminate(const child_handle &p, std::error_code &ec) noexcept {
            if (::kill(p.pid, SIGKILL) == -1)
                ec = get_last_error();
            else
                ec.clear();

            int status;
            ::waitpid(p.pid, &status, WNOHANG); // just to clean it up
        }

        inline void terminate(const child_handle &p) {
            std::error_code ec;
            terminate(p, ec);
            throw_error(ec, "kill(2) failed");
        }

    } // namespace details
} // namespace process

#endif // PROCESS_TERMINATE_HPP
