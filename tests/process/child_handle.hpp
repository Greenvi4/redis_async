#ifndef PROCESS_CHILD_HANDLE_HPP
#define PROCESS_CHILD_HANDLE_HPP

#include <system_error>
#include <utility>

namespace process {
    namespace details {

        typedef ::pid_t pid_t;

        struct child_handle {
            int pid = -1;

            explicit child_handle(int pid)
                : pid(pid) {
            }

            child_handle() = default;
            ~child_handle() = default;
            child_handle(const child_handle &c) = delete;
            child_handle &operator=(const child_handle &c) = delete;

            child_handle(child_handle &&c)
                : pid(c.pid) {
                c.pid = -1;
            }

            child_handle &operator=(child_handle &&c) {
                pid = c.pid;
                c.pid = -1;
                return *this;
            }

            int id() const {
                return pid;
            }

            bool in_group() const {
                return true;
            }

            bool in_group(std::error_code &) const noexcept {
                return true;
            }

            typedef int process_handle_t;

            process_handle_t process_handle() const {
                return pid;
            }

            bool valid() const {
                return pid != -1;
            }
        };

    } // namespace details
} // namespace process

#endif // PROCESS_CHILD_HANDLE_HPP
