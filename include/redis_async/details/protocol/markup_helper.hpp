//
// Created by niko on 06.07.2021.
//

#ifndef REDIS_ASYNC_MARKUP_HELPER_HPP
#define REDIS_ASYNC_MARKUP_HELPER_HPP

#include <redis_async/details/protocol/parser_types.hpp>

#include <boost/lexical_cast.hpp>

namespace redis_async {
    namespace details {

        template <typename Iterator>
        struct markup_helper_t {

            static auto markup_string(size_t consumed, const Iterator &from, const Iterator &to)
                -> parse_result_t {
                return parse_result_t{
                    positive_parse_result_t{result_t{string_t{std::string{from, to}}}, consumed}};
            }

            static auto markup_nil(size_t consumed) -> parse_result_t {
                return parse_result_t{positive_parse_result_t{result_t{nil_t{}}, consumed}};
            }

            static auto markup_error(positive_parse_result_t &wrapped_string) -> parse_result_t {
                auto &str = boost::get<string_t>(wrapped_string.result);
                return parse_result_t{error_t{std::move(str), wrapped_string.consumed}};
            }

            static auto markup_int(positive_parse_result_t &wrapped_string) -> parse_result_t {
                auto &str = boost::get<string_t>(wrapped_string.result);
                return parse_result_t{positive_parse_result_t{
                    result_t{boost::lexical_cast<int_t>(str)}, wrapped_string.consumed}};
            }

            static auto markup_protocol_error(error::errc ec) -> parse_result_t {
                return parse_result_t{protocol_error_t{error::make_error_code(ec)}};
            }
        };

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_MARKUP_HELPER_HPP
