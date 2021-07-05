//
// Created by niko on 01.07.2021.
//

#ifndef REDIS_ASYNC_RD_TYPES_HPP
#define REDIS_ASYNC_RD_TYPES_HPP

#include <boost/variant.hpp>
#include <vector>

namespace redis_async {

    using string_t = std::string;
    using int_t = int64_t;
    struct nil_t {};

    // forward declaration
    struct array_holder_t;
    using array_wrapper_t = boost::recursive_wrapper<array_holder_t>;

    using result_t = boost::variant<int_t, string_t, nil_t, array_wrapper_t>;

    struct array_holder_t {
        using recursive_array_t = std::vector<result_t>;
        recursive_array_t elements;
    };

} // namespace redis_async

#endif // REDIS_ASYNC_RD_TYPES_HPP
