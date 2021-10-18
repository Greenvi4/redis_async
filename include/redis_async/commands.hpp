//
// Created by niko on 29.06.2021.
//

#ifndef REDIS_ASYNC_COMMANDS_HPP
#define REDIS_ASYNC_COMMANDS_HPP

#include <redis_async/command_options.hpp>
#include <redis_async/error.hpp>
#include <variant>
#include <string_view>
#include <chrono>
#include <vector>

namespace redis_async {

    struct single_command_t {
        using args_container_t = std::vector<std::string>;
        args_container_t arguments;

        single_command_t(std::initializer_list<std::string_view> args)
            : arguments(args.begin(), args.end()) {
            if (arguments.empty())
                throw error::client_error("Empty command not allowed");
        }

        explicit single_command_t() = default;
    };

    using command_container_t = std::vector<single_command_t>;
    using command_wrapper_t = std::variant<command_container_t, single_command_t>;

    namespace cmd {
        // ping commands
        single_command_t ping(std::string_view msg = {});
        single_command_t echo(std::string_view msg);

        // key/value commands
        single_command_t set(std::string_view key, std::string_view value);
        single_command_t set(std::string_view key, std::string_view value, UpdateType udp);
        single_command_t set(std::string_view key, std::string_view value, std::chrono::milliseconds ttl);
        single_command_t set(std::string_view key, std::string_view value, UpdateType udp,
                             std::chrono::milliseconds ttl);
        single_command_t get(std::string_view key);
        single_command_t mset(std::initializer_list<std::pair<std::string_view, std::string_view>> kv);
        single_command_t mget(std::initializer_list<std::string_view> keys);
        single_command_t del(std::initializer_list<std::string_view> keys);
        single_command_t exists(std::initializer_list<std::string_view> keys);
        single_command_t expire(std::string_view key, std::chrono::seconds ttl);
        single_command_t pexpire(std::string_view key, std::chrono::milliseconds ttl);
        single_command_t ttl(std::string_view key);
        single_command_t pttl(std::string_view key);
        single_command_t rename(std::string_view key, std::string_view newkey);
        single_command_t keys(std::string_view pattern);

        // hash commands
        single_command_t hset(std::string_view key,
                              std::initializer_list<std::pair<std::string_view, std::string_view>> kv);
        single_command_t hdel(std::string_view key, std::initializer_list<std::string_view> keys);
        single_command_t hget(std::string_view key, std::string_view field);
        single_command_t hkeys(std::string_view key);
        single_command_t hmset(std::string_view key,
                               std::initializer_list<std::pair<std::string_view, std::string_view>> kv);
        single_command_t hmget(std::string_view key, std::initializer_list<std::string_view> fields);

        // list commands
        single_command_t lpush(std::string_view key, std::initializer_list<std::string_view> elements);
        single_command_t rpush(std::string_view key, std::initializer_list<std::string_view> elements);
        single_command_t lpop(std::string_view key);
        single_command_t rpop(std::string_view key);
        single_command_t llen(std::string_view key);
        single_command_t lrange(std::string_view key, int start, int stop);
        single_command_t lset(std::string_view key, int index, std::string_view element);
        single_command_t lrem(std::string_view key, int count, std::string_view element);
        single_command_t lindex(std::string_view key, int index);
        single_command_t ltrim(std::string_view key, int start, int stop);

        // set commands
        single_command_t sadd(std::string_view key, std::initializer_list<std::string_view> members);
        single_command_t scard(std::string_view key);
        single_command_t sdiff(std::initializer_list<std::string_view> keys);
        single_command_t sdiffstore(std::string_view dest, std::initializer_list<std::string_view> keys);
        single_command_t sinter(std::initializer_list<std::string_view> keys);
        single_command_t sinterstore(std::string_view dest, std::initializer_list<std::string_view> keys);
        single_command_t smembers(std::string_view key);
        single_command_t spop(std::string_view key, int count = 0);
        single_command_t srem(std::string_view key, std::initializer_list<std::string_view> members);
        single_command_t sunion(std::initializer_list<std::string_view> keys);
        single_command_t sunionstore(std::string_view dest, std::initializer_list<std::string_view> keys);

    } // namespace cmd

} // namespace redis_async

#endif // REDIS_ASYNC_COMMANDS_HPP
