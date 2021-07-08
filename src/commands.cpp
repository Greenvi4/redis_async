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

        single_command_t ping(StringView msg) {
            if (msg.data())
                return {"PING", msg};
            else
                return {"PING"};
        }

        single_command_t echo(StringView msg) {
            return {"ECHO", msg};
        }

        single_command_t set(StringView key, StringView value) {
            return set(key, value, UpdateType::always, std::chrono::milliseconds(0));
        }
        single_command_t set(StringView key, StringView value, UpdateType udp) {
            return set(key, value, udp, std::chrono::milliseconds(0));
        }
        single_command_t set(StringView key, StringView value, std::chrono::milliseconds ttl) {
            return set(key, value, UpdateType::always, ttl);
        }

        single_command_t set(StringView key, StringView value, UpdateType type,
                             std::chrono::milliseconds ttl) {
            CmdArgs args;
            args << "SET" << key << value;

            if (ttl > std::chrono::milliseconds(0)) {
                args << "PX" << ttl.count();
            }

            details::set_update_type(args, type);
            return std::move(args.cmd());
        }

        single_command_t get(StringView key) {
            return {"GET", key};
        }

    } // namespace cmd
} // namespace redis_async