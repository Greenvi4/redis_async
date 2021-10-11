//
// Created by niko on 07.07.2021.
//

#include <redis_async/commands.hpp>
#include <redis_async/details/protocol/command_args.hpp>

namespace redis_async {
    namespace cmd {

        using details::CmdArgs;

        namespace details {

            void set_update_type(CmdArgs &args, UpdateType type) {
                switch (type) {
                case UpdateType::exist:
                    args << "XX";
                    break;

                case UpdateType::not_exist:
                    args << "NX";
                    break;

                case UpdateType::always:
                    // Do nothing.
                    break;
                default:
                    throw error::client_error("Invalid update type " +
                                              std::to_string(static_cast<int>(type)));
                }
            }

        } // namespace details

        single_command_t ping(std::string_view msg) {
            if (msg.data())
                return {"PING", msg};
            else
                return {"PING"};
        }

        single_command_t echo(std::string_view msg) {
            return {"ECHO", msg};
        }

        single_command_t set(std::string_view key, std::string_view value) {
            return set(key, value, UpdateType::always, std::chrono::milliseconds(0));
        }
        single_command_t set(std::string_view key, std::string_view value, UpdateType udp) {
            return set(key, value, udp, std::chrono::milliseconds(0));
        }
        single_command_t set(std::string_view key, std::string_view value, std::chrono::milliseconds ttl) {
            return set(key, value, UpdateType::always, ttl);
        }

        single_command_t set(std::string_view key, std::string_view value, UpdateType type,
                             std::chrono::milliseconds ttl) {
            CmdArgs args;
            args << "SET" << key << value;

            if (ttl > std::chrono::milliseconds(0)) {
                args << "PX" << ttl.count();
            }

            details::set_update_type(args, type);
            return std::move(args.cmd());
        }

        single_command_t get(std::string_view key) {
            return {"GET", key};
        }

        single_command_t mset(std::initializer_list<std::pair<std::string_view, std::string_view>> kv) {
            if (kv.size() == 0)
                throw error::client_error("MSET could not run without parameters");

            CmdArgs args;
            args << "MSET" << std::make_pair(kv.begin(), kv.end());
            return std::move(args.cmd());
        }

        single_command_t del(std::initializer_list<std::string_view> keys) {
            if (keys.size() == 0)
                throw error::client_error("DEL could not run without parameters");

            CmdArgs args;
            args << "DEL" << std::make_pair(keys.begin(), keys.end());
            return std::move(args.cmd());
        }

        single_command_t exists(std::initializer_list<std::string_view> keys) {
            if (keys.size() == 0)
                throw error::client_error("EXISTS could not run without parameters");

            CmdArgs args;
            args << "EXISTS" << std::make_pair(keys.begin(), keys.end());
            return std::move(args.cmd());
        }

        single_command_t expire(std::string_view key, std::chrono::seconds ttl) {
            CmdArgs args;
            args << "EXPIRE" << key << ttl.count();
            return std::move(args.cmd());
        }

        single_command_t pexpire(std::string_view key, std::chrono::milliseconds ttl) {
            CmdArgs args;
            args << "PEXPIRE" << key << ttl.count();
            return std::move(args.cmd());
        }

        single_command_t ttl(std::string_view key) {
            return {"TTL", key};
        }

        single_command_t pttl(std::string_view key) {
            return {"PTTL", key};
        }

        single_command_t rename(std::string_view key, std::string_view newkey) {
            return {"RENAME", key, newkey};
        }

        single_command_t keys(std::string_view pattern) {
            return {"KEYS", pattern};
        }

        single_command_t mget(std::initializer_list<std::string_view> keys) {
            if (keys.size() == 0)
                throw error::client_error("MGET could not run without parameters");
            CmdArgs args;
            args << "MGET" << std::make_pair(keys.begin(), keys.end());
            return std::move(args.cmd());
        }

        single_command_t hset(std::string_view key,
                              std::initializer_list<std::pair<std::string_view, std::string_view>> kv) {
            if (kv.size() == 0)
                throw error::client_error("HSET could not run without [field, value]");
            CmdArgs args;
            args << "HSET" << key << std::make_pair(kv.begin(), kv.end());
            return std::move(args.cmd());
        }

        single_command_t hdel(std::string_view key, std::initializer_list<std::string_view> keys) {
            if (keys.size() == 0)
                throw error::client_error("HDEL could not run without keys");
            CmdArgs args;
            args << "HDEL" << key << std::make_pair(keys.begin(), keys.end());
            return std::move(args.cmd());
        }

        single_command_t hget(std::string_view key, std::string_view field) {
            return {"HGET", key, field};
        }

        single_command_t hkeys(std::string_view key) {
            return {"HKEYS", key};
        }

        single_command_t hmset(std::string_view key,
                               std::initializer_list<std::pair<std::string_view, std::string_view>> kv) {
            if (kv.size() == 0)
                throw error::client_error("HMSET could not run without fields/values");
            CmdArgs args;
            args << "HMSET" << key << std::make_pair(kv.begin(), kv.end());
            return std::move(args.cmd());
        }

        single_command_t hmget(std::string_view key, std::initializer_list<std::string_view> fields) {
            if (fields.size() == 0)
                throw error::client_error("HMGET could not run without fields");
            CmdArgs args;
            args << "HMGET" << key << std::make_pair(fields.begin(), fields.end());
            return std::move(args.cmd());
        }

        single_command_t lpush(std::string_view key, std::initializer_list<std::string_view> elements) {
            if (elements.size() == 0)
                throw error::client_error("LPUSH could not run without elements");
            CmdArgs args;
            args << "LPUSH" << key << std::make_pair(elements.begin(), elements.end());
            return std::move(args.cmd());
        }

        single_command_t rpush(std::string_view key, std::initializer_list<std::string_view> elements) {
            if (elements.size() == 0)
                throw error::client_error("RPUSH could not run without elements");
            CmdArgs args;
            args << "RPUSH" << key << std::make_pair(elements.begin(), elements.end());
            return std::move(args.cmd());
        }

        single_command_t lpop(std::string_view key) {
            return {"LPOP", key};
        }

        single_command_t rpop(std::string_view key) {
            return {"RPOP", key};
        }

        single_command_t llen(std::string_view key) {
            return {"LLEN", key};
        }

        single_command_t lrange(std::string_view key, int start, int stop) {
            CmdArgs args;
            args << "LRANGE" << key << start << stop;
            return std::move(args.cmd());
        }

        single_command_t lset(std::string_view key, int index, std::string_view element) {
            CmdArgs args;
            args << "LSET" << key << index << element;
            return std::move(args.cmd());
        }

        single_command_t lrem(std::string_view key, int count, std::string_view element) {
            CmdArgs args;
            args << "LREM" << key << count << element;
            return std::move(args.cmd());
        }

        single_command_t lindex(std::string_view key, int index) {
            CmdArgs args;
            args << "LINDEX" << key << index;
            return std::move(args.cmd());
        }

        single_command_t ltrim(std::string_view key, int start, int stop) {
            CmdArgs args;
            args << "LTRIM" << key << start << stop;
            return std::move(args.cmd());
        }

        single_command_t sadd(std::string_view key, std::initializer_list<std::string_view> members) {
            if (members.size() == 0)
                throw error::client_error("SADD could not run without elements");
            CmdArgs args;
            args << "SADD" << key << std::make_pair(members.begin(), members.end());
            return std::move(args.cmd());
        }

        single_command_t scard(std::string_view key) {
            return {"SCARD", key};
        }

        single_command_t sdiff(std::initializer_list<std::string_view> keys) {
            if (keys.size() == 0)
                throw error::client_error("SDIFF could not run without elements");
            CmdArgs args;
            args << "SDIFF" << std::make_pair(keys.begin(), keys.end());
            return std::move(args.cmd());
        }

        single_command_t sdiffstore(std::string_view dest, std::initializer_list<std::string_view> keys) {
            if (keys.size() == 0)
                throw error::client_error("SDIFFSTORE could not run without elements");
            CmdArgs args;
            args << "SDIFFSTORE" << dest << std::make_pair(keys.begin(), keys.end());
            return std::move(args.cmd());
        }

        single_command_t sinter(std::initializer_list<std::string_view> keys) {
            if (keys.size() == 0)
                throw error::client_error("SINTER could not run without elements");
            CmdArgs args;
            args << "SINTER" << std::make_pair(keys.begin(), keys.end());
            return std::move(args.cmd());
        }

        single_command_t sinterstore(std::string_view dest, std::initializer_list<std::string_view> keys) {
            if (keys.size() == 0)
                throw error::client_error("SINTERSTORE could not run without elements");
            CmdArgs args;
            args << "SINTERSTORE" << dest << std::make_pair(keys.begin(), keys.end());
            return std::move(args.cmd());
        }

        single_command_t smembers(std::string_view key) {
            return {"SMEMBERS", key};
        }

        single_command_t spop(std::string_view key, int count) {
            if (count > 0)
                return {"SPOP", key, std::to_string(count)};
            return {"SPOP", key};
        }

        single_command_t srem(std::string_view key, std::initializer_list<std::string_view> members) {
            if (members.size() == 0)
                throw error::client_error("SREM could not run without elements");
            CmdArgs args;
            args << "SREM" << key << std::make_pair(members.begin(), members.end());
            return std::move(args.cmd());
        }

        single_command_t sunion(std::initializer_list<std::string_view> keys) {
            if (keys.size() == 0)
                throw error::client_error("SUNION could not run without elements");
            CmdArgs args;
            args << "SUNION" << std::make_pair(keys.begin(), keys.end());
            return std::move(args.cmd());
        }

        single_command_t sunionstore(std::string_view dest, std::initializer_list<std::string_view> keys) {
            if (keys.size() == 0)
                throw error::client_error("SUNIONSTORE could not run without elements");
            CmdArgs args;
            args << "SUNIONSTORE" << dest << std::make_pair(keys.begin(), keys.end());
            return std::move(args.cmd());
        }

    } // namespace cmd
} // namespace redis_async