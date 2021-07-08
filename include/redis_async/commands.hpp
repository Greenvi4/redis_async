//
// Created by niko on 29.06.2021.
//

#ifndef REDIS_ASYNC_COMMANDS_HPP
#define REDIS_ASYNC_COMMANDS_HPP

#include <redis_async/command_options.hpp>
#include <redis_async/error.hpp>

#include <boost/utility/string_view.hpp>
#include <boost/variant.hpp>
#include <chrono>
#include <vector>

namespace redis_async {

    using StringView = boost::string_view;

    struct single_command_t {

        template <bool...>
        struct bool_pack;
        template <bool... bs>
        using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;
        template <class R, class... Ts>
        using are_all_constructible = all_true<std::is_constructible<R, Ts>::value...>;

        using args_container_t = std::vector<std::string>;
        args_container_t arguments;

        single_command_t(std::initializer_list<StringView> args)
            : arguments(args.begin(), args.end()) {
            if (arguments.empty())
                throw error::client_error("Empty command not allowed");
        }

        explicit single_command_t() = default;
    };

    using command_container_t = std::vector<single_command_t>;
    using command_wrapper_t = boost::variant<command_container_t, single_command_t>;

    namespace cmd {
        single_command_t ping(StringView msg = {});
        single_command_t echo(StringView msg);

        single_command_t set(StringView key, StringView value);
        single_command_t set(StringView key, StringView value, UpdateType udp);
        single_command_t set(StringView key, StringView value, std::chrono::milliseconds ttl);
        single_command_t set(StringView key, StringView value, UpdateType udp,
                             std::chrono::milliseconds ttl);

        single_command_t get(StringView key);
    } // namespace cmd

} // namespace redis_async

#endif // REDIS_ASYNC_COMMANDS_HPP
