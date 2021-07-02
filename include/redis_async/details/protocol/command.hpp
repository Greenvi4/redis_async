//
// Created by niko on 29.06.2021.
//

#ifndef REDIS_ASYNC_COMMAND_HPP
#define REDIS_ASYNC_COMMAND_HPP

#include <boost/utility/string_ref.hpp>
#include <boost/variant.hpp>
#include <vector>

namespace redis_async {
    namespace details {

        struct single_command_t {

            template <bool...>
            struct bool_pack;
            template <bool... bs>
            using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;

            template <class R, class... Ts>
            using are_all_constructible = all_true<std::is_constructible<R, Ts>::value...>;

            using args_container_t = std::vector<boost::string_ref>;
            args_container_t arguments;

            template <typename... Args, typename = std::enable_if_t<are_all_constructible<
                                            boost::string_ref, Args...>::value>>
            single_command_t(Args &&...args)
                : arguments{std::forward<Args>(args)...} {
                static_assert(sizeof...(Args) >= 1, "Empty command is not allowed");
            }
        };

        using command_container_t = std::vector<single_command_t>;
        using command_wrapper_t = boost::variant<command_container_t, single_command_t>;


    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_COMMAND_HPP
