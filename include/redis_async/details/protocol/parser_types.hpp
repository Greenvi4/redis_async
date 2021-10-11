//
// Created by niko on 06.07.2021.
//

#ifndef REDIS_ASYNC_PARSER_TYPES_HPP
#define REDIS_ASYNC_PARSER_TYPES_HPP

#include <redis_async/rd_types.hpp>

namespace redis_async {
    namespace details {

        struct protocol_error_t {
            std::error_code code;
        };

        struct error_t {
            string_t str;
            size_t consumed;
        };

        struct positive_parse_result_t {
            result_t result;
            size_t consumed;
        };

        using parse_result_t = std::variant<protocol_error_t, error_t, positive_parse_result_t>;

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_PARSER_TYPES_HPP
