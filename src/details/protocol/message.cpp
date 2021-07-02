//
// Created by niko on 29.06.2021.
//

#include <redis_async/details/protocol/message.hpp>
#include <boost/asio/buffer.hpp>

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
                using namespace boost::asio;
                char data[buff_sz];
                std::size_t total = snprintf(data, buff_sz, "*%zu\r\n", cmd.arguments.size());
                auto it = std::copy(data, data + total, std::back_inserter(buff));

                for (const auto &arg : cmd.arguments) {
                    auto bytes = snprintf(data, buff_sz, "$%zu\r\n", arg.size());
                    it = std::copy(data, data + bytes, it);
                    total += bytes;

                    if (!arg.empty()) {
                        it = std::copy(arg.begin(), arg.end(), it);
                        total += arg.size();
                    }
                    it = '\r';
                    it = '\n';
                    total += terminator_size;
                }
                //                buff.commit(total);
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

            void operator()(const single_command_t &value) const {
                Protocol::serialize(buff_, value);
            }

            void operator()(const command_container_t &value) const {
                for (const auto &cmd : value) {
                    Protocol::serialize(buff_, cmd);
                }
            }

        private:
        };

        message::message(const command_wrapper_t &command) {
            payload.reserve(256);
            using serializer_t = command_serializer_visitor<buffer_type>;
            boost::apply_visitor(serializer_t(payload), command);
        }

        message::message(message &&other) noexcept
            : payload{::std::move(other.payload)} {
        }

        std::size_t message::size() const {
            return payload.size();
        }

        message::const_range message::buffer() const {
            return std::make_pair(payload.begin(), payload.end());
        }

    } // namespace details
} // namespace redis_async
