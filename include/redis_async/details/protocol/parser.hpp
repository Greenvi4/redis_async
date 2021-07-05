//
// Created by niko on 02.07.2021.
//

#ifndef REDIS_ASYNC_PARSER_HPP
#define REDIS_ASYNC_PARSER_HPP

#include <redis_async/error.hpp>
#include <redis_async/rd_types.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>

namespace redis_async {
    namespace details {

        static std::string terminator = "\r\n";

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

        using parse_result_t = boost::variant<protocol_error_t, error_t, positive_parse_result_t>;

        template <typename Iterator>
        parse_result_t raw_parse(const Iterator &from, const Iterator &to);

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
        };

        template <typename Iterator>
        struct string_parser_t {
            static parse_result_t apply(const Iterator &from, const Iterator &to,
                                        std::size_t already_consumed) {
                using helper = markup_helper_t<Iterator>;

                auto found_terminator = std::search(from, to, terminator.begin(), terminator.end());
                if (found_terminator == to)
                    return protocol_error_t{error::make_error_code(error::errc::not_enough_data)};
                size_t consumed =
                    terminator.size() + std::distance(from, found_terminator) + already_consumed;
                return helper::markup_string(consumed, from, found_terminator);
            }
        };

        template <typename Iterator>
        struct error_parser_t {
            static parse_result_t apply(const Iterator &from, const Iterator &to,
                                        std::size_t already_consumed) {
                using helper = markup_helper_t<Iterator>;
                using parser_t = string_parser_t<Iterator>;

                auto result = parser_t::apply(from, to, already_consumed);
                auto *wrapped_string = boost::get<positive_parse_result_t>(&result);
                if (!wrapped_string) {
                    return result;
                }
                return helper::markup_error(*wrapped_string);
            }
        };

        template <typename Iterator>
        struct int_parser_t {
            static parse_result_t apply(const Iterator &from, const Iterator &to,
                                        std::size_t already_consumed) {
                using helper = markup_helper_t<Iterator>;
                using parser_t = string_parser_t<Iterator>;

                auto result = parser_t::apply(from, to, already_consumed);
                auto *wrapped_string = boost::get<positive_parse_result_t>(&result);
                if (!wrapped_string) {
                    return result;
                }
                return helper::markup_int(*wrapped_string);
            }
        };

        template <typename Iterator>
        struct bulk_string_parser_t {
            static parse_result_t apply(const Iterator &from, const Iterator &to,
                                        std::size_t already_consumed) {
                using helper = markup_helper_t<Iterator>;
                using count_parser_t = int_parser_t<Iterator>;
                auto count_result = count_parser_t::apply(from, to, already_consumed);
                auto *count_wrapped = boost::get<positive_parse_result_t>(&count_result);
                if (!count_wrapped) {
                    return count_result;
                }
                auto head = from + (count_wrapped->consumed - already_consumed);
                size_t left = std::distance(head, to);
                auto count = boost::get<int_t>(count_wrapped->result);
                if (count == -1)
                    return helper::markup_nil(count_wrapped->consumed);
                else if (count < -1)
                    return protocol_error_t{error::make_error_code(error::errc::count_range)};

                auto terminator_size = terminator.size();
                if (left < count + terminator_size) {
                    return protocol_error_t{error::make_error_code(error::errc::not_enough_data)};
                }
                auto tail = head + count;
                auto tail_end = tail + terminator_size;
                bool found_terminator =
                    std::equal(tail, tail_end, terminator.begin(), terminator.end());
                if (!found_terminator) {
                    return protocol_error_t{error::make_error_code(error::errc::bulk_terminator)};
                }
                size_t consumed = count_wrapped->consumed + count + terminator_size;
                return helper::markup_string(consumed, head, tail);
            }
        };

        template <typename Iterator>
        struct array_parser_t {
            static parse_result_t apply(const Iterator &from, const Iterator &to,
                                        std::size_t already_consumed) {
                using helper = markup_helper_t<Iterator>;
                using count_parser_t = int_parser_t<Iterator>;
                using element_t = positive_parse_result_t;
                auto count_result = count_parser_t::apply(from, to, already_consumed);
                auto *count_wrapped = boost::get<positive_parse_result_t>(&count_result);
                if (!count_wrapped) {
                    return count_result;
                }
                auto count = boost::get<int_t>(count_wrapped->result);
                if (count == -1)
                    return helper::markup_nil(count_wrapped->consumed);
                else if (count < -1)
                    return protocol_error_t{error::make_error_code(error::errc::count_range)};

                array_holder_t array;
                array.elements.reserve(count);
                Iterator element_from = from + (count_wrapped->consumed - already_consumed);
                std::size_t consumed = count_wrapped->consumed;

                while (count) {
                    auto element_result = raw_parse<Iterator>(element_from, to);
                    auto *element = boost::get<element_t>(&element_result);
                    if (!element) {
                        return element_result;
                    }
                    element_from += element->consumed;
                    array.elements.push_back(std::move(element->result));
                    consumed += element->consumed;
                    --count;
                }

                return positive_parse_result_t{std::move(array), consumed};
            }
        };

        template <typename Iterator>
        using primary_parser_t =
            boost::variant<protocol_error_t, string_parser_t<Iterator>, int_parser_t<Iterator>,
                           error_parser_t<Iterator>, bulk_string_parser_t<Iterator>,
                           array_parser_t<Iterator>>;

        template <typename Iterator>
        struct unwrap_primary_parser_t : public boost::static_visitor<parse_result_t> {
            using wrapped_result_t = parse_result_t;

            const Iterator &from_;
            const Iterator &to_;

            unwrap_primary_parser_t(const Iterator &from, const Iterator &to)
                : from_{from}
                , to_{to} {
            }

            wrapped_result_t operator()(const protocol_error_t &value) const {
                return value;
            }

            template <typename Parser>
            wrapped_result_t operator()(const Parser & /*ignored*/) const {
                return Parser::apply(std::next(from_), to_, 1);
            }
        };

        template <typename Iterator>
        struct construct_primary_parser_t {
            using result_t = primary_parser_t<Iterator>;

            static auto apply(const Iterator &from, const Iterator &to) -> result_t {
                if (from == to) {
                    return protocol_error_t{error::make_error_code(error::errc::not_enough_data)};
                }

                switch (*from) {
                case '+':
                    return string_parser_t<Iterator>{};
                case '-':
                    return error_parser_t<Iterator>{};
                case ':':
                    return int_parser_t<Iterator>{};
                case '$':
                    return bulk_string_parser_t<Iterator>{};
                case '*':
                    return array_parser_t<Iterator>{};
                }
                // wrong introduction;
                return protocol_error_t{error::make_error_code(error::errc::wrong_introduction)};
            }
        };

        template <typename Iterator>
        parse_result_t raw_parse(const Iterator &from, const Iterator &to) {
            auto primary = construct_primary_parser_t<Iterator>::apply(from, to);
            return boost::apply_visitor(unwrap_primary_parser_t<Iterator>(from, to), primary);
        }

    } // namespace details
} // namespace redis_async

#endif // REDIS_ASYNC_PARSER_HPP
