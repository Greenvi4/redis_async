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
        single_command_t mset(std::initializer_list<std::pair<StringView, StringView>> kv);
        single_command_t mget(std::initializer_list<StringView> keys);
        single_command_t del(std::initializer_list<StringView> keys);
        single_command_t exists(std::initializer_list<StringView> keys);
        single_command_t expire(StringView key, std::chrono::seconds ttl);
        single_command_t pexpire(StringView key, std::chrono::milliseconds ttl);
        single_command_t ttl(StringView key);
        single_command_t pttl(StringView key);
        single_command_t rename(StringView key, StringView newkey);

        single_command_t hset(StringView key,
                              std::initializer_list<std::pair<StringView, StringView>> kv);
        single_command_t hdel(StringView key, std::initializer_list<StringView> keys);
        single_command_t hget(StringView key, StringView field);
        single_command_t hkeys(StringView key);
        single_command_t hmset(StringView key,
                               std::initializer_list<std::pair<StringView, StringView>> kv);
        single_command_t hmget(StringView key, std::initializer_list<StringView> fields);

        single_command_t lpush(StringView key, std::initializer_list<StringView> elements);
        single_command_t rpush(StringView key, std::initializer_list<StringView> elements);
        single_command_t lpop(StringView key);
        single_command_t rpop(StringView key);
        single_command_t llen(StringView key);
        single_command_t lrange(StringView key, int start, int stop);
        single_command_t lset(StringView key, int index, StringView element);
        single_command_t lrem(StringView key, int count, StringView element);
        single_command_t lindex(StringView key, int index);
        single_command_t ltrim(StringView key, int start, int stop);


    } // namespace cmd

} // namespace redis_async

#endif // REDIS_ASYNC_COMMANDS_HPP
