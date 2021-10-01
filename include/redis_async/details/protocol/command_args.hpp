//
// Created by niko on 07.07.2021.
//

#ifndef REDIS_ASYNC_COMMAND_ARGS_HPP
#define REDIS_ASYNC_COMMAND_ARGS_HPP

#include <redis_async/commands.hpp>

namespace redis_async {
    namespace cmd {

        namespace details {

            template <typename...>
            struct IsKvPair : std::false_type {};

            template <typename T, typename U>
            struct IsKvPair<std::pair<T, U>> : std::true_type {};

            class CmdArgs {
            public:
                template <typename Arg>
                CmdArgs &append(Arg &&arg);

                template <typename Arg, typename... Args>
                CmdArgs &append(Arg &&arg, Args &&...args);

                // All overloads of operator<< are for internal use only.
                CmdArgs &operator<<(const std::string_view &arg);

                template <typename T, typename std::enable_if<
                                          std::is_arithmetic<typename std::decay<T>::type>::value,
                                          int>::type = 0>
                CmdArgs &operator<<(T &&arg);

                template <typename Iter>
                CmdArgs &operator<<(const std::pair<Iter, Iter> &range);

                template <std::size_t N, typename... Args>
                auto operator<<(const std::tuple<Args...> &) ->
                    typename std::enable_if<N == sizeof...(Args), CmdArgs &>::type {
                    return *this;
                }

                template <std::size_t N = 0, typename... Args>
                    auto operator<<(const std::tuple<Args...> &arg) ->
                    typename std::enable_if < N<sizeof...(Args), CmdArgs &>::type;

                single_command_t &cmd() { return m_cmd; }

            private:
                // Deep copy.
                CmdArgs &_append(std::string arg);

                // Shallow copy.
                CmdArgs &_append(const std::string_view &arg);

                // Shallow copy.
                CmdArgs &_append(const char *arg);

                template <typename T, typename std::enable_if<
                                          std::is_arithmetic<typename std::decay<T>::type>::value,
                                          int>::type = 0>
                CmdArgs &_append(T &&arg) {
                    return operator<<(std::forward<T>(arg));
                }

                template <typename Iter>
                CmdArgs &_append(std::true_type, const std::pair<Iter, Iter> &range);

                template <typename Iter>
                CmdArgs &_append(std::false_type, const std::pair<Iter, Iter> &range);

                single_command_t m_cmd;
            };

            template <typename Arg>
            inline CmdArgs &CmdArgs::append(Arg &&arg) {
                return _append(std::forward<Arg>(arg));
            }

            template <typename Arg, typename... Args>
            inline CmdArgs &CmdArgs::append(Arg &&arg, Args &&...args) {
                _append(std::forward<Arg>(arg));

                return append(std::forward<Args>(args)...);
            }

            inline CmdArgs &CmdArgs::operator<<(const std::string_view &arg) {
                m_cmd.arguments.emplace_back(arg.data(), arg.size());
                return *this;
            }

            template <typename Iter>
            inline CmdArgs &CmdArgs::operator<<(const std::pair<Iter, Iter> &range) {
                return _append(
                    IsKvPair<typename std::decay<decltype(*std::declval<Iter>())>::type>(), range);
            }

            template <typename T,
                      typename std::enable_if<
                          std::is_arithmetic<typename std::decay<T>::type>::value, int>::type>
            inline CmdArgs &CmdArgs::operator<<(T &&arg) {
                return _append(std::to_string(std::forward<T>(arg)));
            }

            template <std::size_t N, typename... Args>
                auto CmdArgs::operator<<(const std::tuple<Args...> &arg) ->
                typename std::enable_if < N<sizeof...(Args), CmdArgs &>::type {
                operator<<(std::get<N>(arg));

                return operator<<<N + 1, Args...>(arg);
            }

            inline CmdArgs &CmdArgs::_append(std::string arg) {
                m_cmd.arguments.emplace_back(std::move(arg));
                return *this;
            }

            inline CmdArgs &CmdArgs::_append(const std::string_view &arg) {
                return operator<<(arg);
            }

            inline CmdArgs &CmdArgs::_append(const char *arg) {
                return operator<<(arg);
            }

            template <typename Iter>
            CmdArgs &CmdArgs::_append(std::false_type, const std::pair<Iter, Iter> &range) {
                auto first = range.first;
                auto last = range.second;
                while (first != last) {
                    *this << *first;
                    ++first;
                }

                return *this;
            }

            template <typename Iter>
            CmdArgs &CmdArgs::_append(std::true_type, const std::pair<Iter, Iter> &range) {
                auto first = range.first;
                auto last = range.second;
                while (first != last) {
                    *this << first->first << first->second;
                    ++first;
                }

                return *this;
            }

        } // namespace details

    } // namespace cmd
} // namespace redis_async

#endif // REDIS_ASYNC_COMMAND_ARGS_HPP
