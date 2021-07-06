#ifndef PROCESS_EXECUTOR_HPP
#define PROCESS_EXECUTOR_HPP

#include "child_decl.hpp"
#include "config.hpp"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>

#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace process {
    namespace details {

        class executor {
            int _pipe_sink = -1;

            void write_error(std::error_code ec, const char *msg) {
                // I am the child
                int len = ec.value();
                auto res = ::write(_pipe_sink, &len, sizeof(int));
                static_cast<void>(res);
                len = std::strlen(msg) + 1;
                res = ::write(_pipe_sink, &len, sizeof(int));
                static_cast<void>(res);
                res = ::write(_pipe_sink, msg, len);
                static_cast<void>(res);
            }

            void internal_error_handle(std::error_code ec, const char *msg) {
                this->_ec = ec;
                this->_msg = msg;
                if (this->pid == 0)
                    write_error(ec, msg);
                else
                    throw process_error(ec, msg);
            }

            void check_error() {
                if (_ec)
                    throw process_error(_ec, _msg);
            }

            inline child invoke();
            void _write_error(int sink) {
                int data[2] = {_ec.value(), static_cast<int>(_msg.size())};
                while (::write(sink, &data[0], sizeof(int) * 2) == -1) {
                    auto err = errno;

                    if (err == EBADF)
                        return;
                    else if ((err != EINTR) && (err != EAGAIN))
                        break;
                }
                while (::write(sink, &_msg.front(), _msg.size()) == -1) {
                    auto err = errno;

                    if (err == EBADF)
                        return;
                    else if ((err != EINTR) && (err != EAGAIN))
                        break;
                }
            }

            void _read_error(int source) {
                int data[2];

                _ec.clear();
                int count = 0;
                while ((count = ::read(source, &data[0], sizeof(int) * 2)) == -1) {
                    // actually, this should block until it's read.
                    auto err = errno;
                    if ((err != EAGAIN) && (err != EINTR))
                        set_error(std::error_code(err, std::system_category()), "Error read pipe");
                }
                if (count == 0)
                    return;

                std::error_code ec(data[0], std::system_category());
                std::string msg(data[1], ' ');

                while (::read(source, &msg.front(), msg.size()) == -1) {
                    // actually, this should block until it's read.
                    auto err = errno;
                    if ((err == EBADF) ||
                        (err == EPERM)) // that should occur on success, therefore return.
                        return;
                    // EAGAIN not yet forked, EINTR interrupted, i.e. try again
                    else if ((err != EAGAIN) && (err != EINTR))
                        set_error(std::error_code(err, std::system_category()), "Error read pipe");
                }
                set_error(ec, std::move(msg));
            }

            std::string prepare_cmd_style_fn; // buffer

            // this does what execvpe does - but we execute it in
            // the father process, to avoid allocations.
            inline void prepare_cmd_style() {
                // use my own implementation
                prepare_cmd_style_fn = exe;
                if ((prepare_cmd_style_fn.find('/') == std::string::npos) &&
                    ::access(prepare_cmd_style_fn.c_str(), X_OK)) {
                    auto e = ::environ;
                    while ((e != nullptr) && (*e != nullptr) && !boost::starts_with(*e, "PATH="))
                        e++;

                    if ((e != nullptr) && (*e != nullptr)) {
                        std::vector<std::string> path;
                        boost::split(path, *e, boost::is_any_of(":"));

                        for (const std::string &pp : path) {
                            auto p = pp + "/" + exe;
                            if (!::access(p.c_str(), X_OK)) {
                                prepare_cmd_style_fn = p;
                                break;
                            }
                        }
                    }
                }
                exe = prepare_cmd_style_fn.c_str();
            }

            std::error_code _ec;
            std::string _msg;

        public:
            executor(std::string str) {
                boost::algorithm::split(seq, str, boost::is_any_of(" "),
                                        boost::algorithm::token_compress_on);
                BOOST_ASSERT(!seq.empty());
                exe = seq.front().c_str();
                cmd_impl.reserve(seq.size());

                if (!seq.empty()) {
                    for (auto & v : seq)
                        cmd_impl.push_back(&v.front());
                }
                cmd_impl.push_back(nullptr);
                cmd_line = cmd_impl.data();
            }

            child operator()() {
                return invoke();
            }

            std::vector<std::string> seq;
            std::vector<char*> cmd_impl;

            const char *exe = nullptr;
            char *const *cmd_line = nullptr;
            bool cmd_style = false;
            char **env = ::environ;
            pid_t pid = -1;
            std::shared_ptr<std::atomic<int>> exit_status =
                std::make_shared<std::atomic<int>>(still_active);

            const std::error_code &error() const {
                return _ec;
            }

            void set_error(std::error_code ec, const char *msg) {
                internal_error_handle(ec, msg);
            }
            void set_error(std::error_code ec, const std::string &msg) {
                set_error(ec, msg.c_str());
            };
        };

        inline child executor::invoke() {
            {
                struct pipe_guard {
                    int p[2];
                    pipe_guard()
                        : p{-1, -1} {
                    }

                    ~pipe_guard() {
                        if (p[0] != -1)
                            ::close(p[0]);
                        if (p[1] != -1)
                            ::close(p[1]);
                    }
                } p{};

                if (::pipe(p.p) == -1) {
                    set_error(get_last_error(), "pipe(2) failed");
                    return child();
                }
                if (::fcntl(p.p[1], F_SETFD, FD_CLOEXEC) == -1) {
                    auto err = get_last_error();
                    set_error(err, "fcntl(2) failed"); // this might throw, so we need to be sure
                                                       // our pipe is safe.
                    return child();
                }
                _ec.clear();

                prepare_cmd_style();

                this->pid = ::fork();
                if (pid == -1) {
                    _ec = get_last_error();
                    _msg = "fork() failed";
                    return child();
                } else if (pid == 0) {
                    _pipe_sink = p.p[1];
                    ::close(p.p[0]);

                    ::execve(exe, cmd_line, env);
                    _ec = get_last_error();
                    _msg = "execve failed";

                    _write_error(_pipe_sink);
                    ::close(p.p[1]);

                    _exit(EXIT_FAILURE);
                    return child();
                }

                ::close(p.p[1]);
                p.p[1] = -1;
                _read_error(p.p[0]);
            }
            if (_ec) {
                return child();
            }

            child c(child_handle(pid), exit_status);

            if (_ec) {
                return child();
            }

            return c;
        }

        template <typename... Args>
        inline executor make_executor(Args &&...args) {
            return executor(std::forward<Args>(args)...);
        }

    } // namespace details
} // namespace process

#endif // PROCESS_EXECUTOR_HPP
