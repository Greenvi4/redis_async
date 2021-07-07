//
// Created by niko on 07.07.2021.
//

#ifndef REDIS_ASYNC_SERIALIZER_HPP
#define REDIS_ASYNC_SERIALIZER_HPP

#include <redis_async/details/protocol/command.hpp>

namespace redis_async {
    namespace details {

        struct Protocol {

            static constexpr std::size_t terminator_size = 2;

            inline static std::size_t size_for_int(std::size_t arg) {
                std::size_t r = 0;
                while (arg) {
                    ++r;
                    arg /= 10;
                }
                // if r == 0 we clamp max to 1 to allow minimum containing "0" as char
                return std::max<size_t>(1, r);
            }

            inline static std::size_t command_size(const single_command_t &cmd) {
                std::size_t sz = 1                                    /* * */
                                 + size_for_int(cmd.arguments.size()) /* args size */
                                 + terminator_size;

                for (const auto &arg : cmd.arguments) {
                    sz += 1                          /* $ */
                          + size_for_int(arg.size()) /* argument size */
                          + terminator_size + arg.size() + terminator_size;
                }
                return sz;
            }

            template <typename DynamicBuffer>
            inline static void serialize(DynamicBuffer &buff, const single_command_t &cmd) {
                buff.reserve(command_size(cmd));
                constexpr std::size_t buff_sz = 64;
                char data[buff_sz];
                std::size_t total = snprintf(data, buff_sz, "*%zu\r\n", cmd.arguments.size());
                auto it = std::copy(data, data + total, std::back_inserter(buff));

                for (const auto &arg : cmd.arguments) {
                    auto bytes = snprintf(data, buff_sz, "$%zu\r\n", arg.size());
                    std::copy(data, data + bytes, it);
                    total += bytes;

                    if (!arg.empty()) {
                        std::copy(arg.begin(), arg.end(), it);
                        total += arg.size();
                    }
                    it = '\r';
                    it = '\n';
                    total += terminator_size;
                }
            }

            template <typename DynamicBuffer>
            inline static void serialize(DynamicBuffer &buff, const command_container_t &cont) {
                for (const auto &cmd : cont) {
                    Protocol::serialize(buff, cmd);
                }
            }
        };

        template <typename DynamicBuffer>
        class command_serializer_visitor : public boost::static_visitor<void> {
        private:
            DynamicBuffer &buff_;

        public:
            command_serializer_visitor(DynamicBuffer &buff)
                : buff_{buff} {
            }

            template<typename T>
            void operator()(const T &value) const {
                Protocol::serialize(buff_, value);
            }
        };

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_SERIALIZER_HPP
