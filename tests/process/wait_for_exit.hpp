#ifndef PROCESS_WAIT_FOR_EXIT_HPP
#define PROCESS_WAIT_FOR_EXIT_HPP

#include "child_handle.hpp"
#include "config.hpp"

#include <chrono>
#include <sys/wait.h>

namespace process {
    namespace details {

        inline void wait(const child_handle &p, int &exit_code, std::error_code &ec) noexcept {
            pid_t ret;
            int status;

            do {
                ret = ::waitpid(p.pid, &status, 0);
            } while (((ret == -1) && (errno == EINTR)) ||
                     (ret != -1 && !WIFEXITED(status) && !WIFSIGNALED(status)));

            if (ret == -1)
                ec = get_last_error();
            else {
                ec.clear();
                exit_code = status;
            }
        }

        inline void wait(const child_handle &p, int &exit_code) noexcept {
            std::error_code ec;
            wait(p, exit_code, ec);
            throw_error(ec, "waitpid(2) failed in wait");
        }

        template <class Clock, class Duration>
        inline bool wait_until(const child_handle &p, int &exit_code,
                               const std::chrono::time_point<Clock, Duration> &time_out,
                               std::error_code &ec) noexcept {
            ::sigset_t sigset;

            // I need to set the signal, because it might be ignore / default, in which case sigwait
            // might not work.

            using _signal_t = void (*)(int);
            static thread_local _signal_t sigchld_handler = SIG_DFL;

            struct signal_interceptor_t {
                static void handler_func(int val) {
                    if ((sigchld_handler != SIG_DFL) && (sigchld_handler != SIG_IGN))
                        sigchld_handler(val);
                }
                signal_interceptor_t() {
                    sigchld_handler = ::signal(SIGCHLD, &handler_func);
                }
                ~signal_interceptor_t() {
                    ::signal(SIGCHLD, sigchld_handler);
                    sigchld_handler = SIG_DFL;
                }

            } signal_interceptor{};

            if (sigemptyset(&sigset) != 0) {
                ec = get_last_error();
                return false;
            }
            if (sigaddset(&sigset, SIGCHLD) != 0) {
                ec = get_last_error();
                return false;
            }

            auto get_timespec = [](const Duration &dur) {
                ::timespec ts;
                ts.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(dur).count();
                ts.tv_nsec =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count() % 1000000000;
                return ts;
            };

            int ret;
            int status{0};

            struct ::sigaction old_sig;
            if (-1 == ::sigaction(SIGCHLD, nullptr, &old_sig)) {
                ec = get_last_error();
                return false;
            }

            bool timed_out;

            do {
                auto ts = get_timespec(time_out - Clock::now());
                auto ret_sig = ::sigtimedwait(&sigset, nullptr, &ts);
                errno = 0;
                ret = ::waitpid(p.pid, &status, WNOHANG);

                if ((ret_sig == SIGCHLD) && (old_sig.sa_handler != SIG_DFL) &&
                    (old_sig.sa_handler != SIG_IGN))
                    old_sig.sa_handler(ret);

                if (ret == 0) {
                    timed_out = Clock::now() >= time_out;
                    if (timed_out)
                        return false;
                }
            } while ((ret == 0) || (((ret == -1) && errno == EINTR) ||
                                    ((ret != -1) && !WIFEXITED(status) && !WIFSIGNALED(status))));

            if (ret == -1)
                ec = get_last_error();
            else {
                ec.clear();
                exit_code = status;
            }

            return true;
        }

        template <class Clock, class Duration>
        inline bool wait_until(const child_handle &p, int &exit_code,
                               const std::chrono::time_point<Clock, Duration> &time_out) {
            std::error_code ec;
            bool b = wait_until(p, exit_code, time_out, ec);
            throw_error(ec, "waitpid(2) failed in wait_until");
            return b;
        }

        template <class Rep, class Period>
        inline bool wait_for(const child_handle &p, int &exit_code,
                             const std::chrono::duration<Rep, Period> &rel_time,
                             std::error_code &ec) noexcept {
            return wait_until(p, exit_code, std::chrono::steady_clock::now() + rel_time, ec);
        }

        template <class Rep, class Period>
        inline bool wait_for(const child_handle &p, int &exit_code,
                             const std::chrono::duration<Rep, Period> &rel_time) {
            std::error_code ec;
            bool b = wait_for(p, exit_code, rel_time, ec);
            throw_error(ec, "waitpid(2) failed in wait_for");
            return b;
        }

    } // namespace details
} // namespace process

#endif // PROCESS_WAIT_FOR_EXIT_HPP