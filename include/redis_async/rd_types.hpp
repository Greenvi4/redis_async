//
// Created by niko on 01.07.2021.
//

#ifndef REDIS_ASYNC_RD_TYPES_HPP
#define REDIS_ASYNC_RD_TYPES_HPP

#include <iostream>
#include <variant>
#include <vector>

namespace redis_async {

    using string_t = std::string;
    using int_t = int64_t;
    struct nil_t {
        friend std::ostream &operator<<(std::ostream &out, const redis_async::nil_t &) {
            out << "{NIL_t}";
            return out;
        }
    };

    // forward declaration
    struct array_holder_t;
    using result_t = std::variant<int_t, string_t, nil_t, array_holder_t>;

    struct array_holder_t {
        using recursive_array_t = std::vector<result_t>;
        recursive_array_t elements;
        friend std::ostream &operator<<(std::ostream &out, const redis_async::array_holder_t &ah) {
            out << "{ARRAY_t}\n";
            for (const auto &item : ah.elements)
                std::visit([&out](const auto &v) { out << '\t' << v << '\n'; }, item);
            return out;
        }
    };

} // namespace redis_async

#endif // REDIS_ASYNC_RD_TYPES_HPP
