//
// Created by niko on 02.07.2021.
//
#include <redis_async/commands.hpp>
#include <redis_async/details/protocol/serializer.hpp>

#include <redis_async/details/protocol/parser.hpp>

#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/streambuf.hpp>

#include <gtest/gtest.h>

TEST(ParserTests, raw_cmd) {
    using redis_async::command_container_t;
    using redis_async::single_command_t;
    using Buffer = std::string;
    using Protocol = redis_async::details::Protocol;

    {
        single_command_t ping{"PING", "Hello, World!"};
        Buffer result;
        Protocol::serialize(result, ping);
        const std::string expected = "*2\r\n$4\r\nPING\r\n$13\r\nHello, World!\r\n";
        ASSERT_EQ(result.size(), expected.size());
        ASSERT_EQ(result, expected);
    }
    {
        single_command_t cmd= {"HSET", "key", "value1", "", "value2", ""};
        Buffer result;
        Protocol::serialize(result, cmd);
        const std::string expected("*6\r\n$4\r\nHSET\r\n$3\r\nkey\r\n$6\r\nvalue1\r\n$0\r\n\r\n$"
                                   "6\r\nvalue2\r\n$0\r\n\r\n");
        ASSERT_EQ(result.size(), expected.size());
        ASSERT_EQ(result, expected);
    }
    {
        command_container_t cont = {{"PING", "Hello, World!"}, {"LPUSH", "list", "value"}};
        Buffer result;
        Protocol::serialize(result, cont);
        const std::string expected = "*2\r\n$4\r\nPING\r\n$13\r\nHello, World!\r\n"
                                     "*3\r\n$5\r\nLPUSH\r\n$4\r\nlist\r\n$5\r\nvalue\r\n";
        ASSERT_EQ(result.size(), expected.size());
        ASSERT_EQ(result, expected);
    }
}

TEST(ParserTests, simple_str) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "+OK\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    ASSERT_EQ("OK", boost::get<redis_async::string_t>(positive_parse_result.result));

    buff.consume(positive_parse_result.consumed);
}

TEST(ParserTests, simple_str_protocol_error) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "+OK\r";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto error = boost::get<redis_async::details::protocol_error_t>(parsed_result);

    ASSERT_EQ(error.code,
              redis_async::error::make_error_code(redis_async::error::errc::not_enough_data));
}

TEST(ParserTests, error) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "-Some Error\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto error = boost::get<redis_async::details::error_t>(parsed_result);

    ASSERT_EQ(answer.size(), error.consumed);
    ASSERT_EQ("Some Error", error.str);

    buff.consume(error.consumed);
}

TEST(ParserTests, integer) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = ":-555423\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    ASSERT_EQ(-555423, boost::get<redis_async::int_t>(positive_parse_result.result));
}

TEST(ParserTests, bulk_string) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "$4\r\nsome\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    ASSERT_EQ("some", boost::get<redis_async::string_t>(positive_parse_result.result));
}

TEST(ParserTests, bulk_string_emply) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "$0\r\n\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    ASSERT_EQ("", boost::get<redis_async::string_t>(positive_parse_result.result));
}

TEST(ParserTests, nil) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "$-1\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    ASSERT_NO_THROW(boost::get<redis_async::nil_t>(positive_parse_result.result));
}

TEST(ParserTests, array) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "*2\r\n$4\r\nsome\r\n:-555423\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    auto array = boost::get<redis_async::array_holder_t>(positive_parse_result.result);
    ASSERT_EQ(array.elements.size(), 2);
    ASSERT_EQ("some", boost::get<redis_async::string_t>(array.elements[0]));
    ASSERT_EQ(-555423, boost::get<redis_async::int_t>(array.elements[1]));
}

TEST(ParserTests, array_empty) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "*0\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    auto array = boost::get<redis_async::array_holder_t>(positive_parse_result.result);
    ASSERT_EQ(array.elements.size(), 0);
}

TEST(ParserTests, array_nil) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "*-1\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    ASSERT_NO_THROW(boost::get<redis_async::nil_t>(positive_parse_result.result));
}

TEST(ParserTests, array_with_nil_element) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "*3\r\n$4\r\nsome\r\n$-1\r\n:-555423\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    auto array = boost::get<redis_async::array_holder_t>(positive_parse_result.result);
    ASSERT_EQ(array.elements.size(), 3);
    ASSERT_EQ("some", boost::get<redis_async::string_t>(array.elements[0]));
    ASSERT_NO_THROW(boost::get<redis_async::nil_t>(array.elements[1]));
    ASSERT_EQ(-555423, boost::get<redis_async::int_t>(array.elements[2]));
}

TEST(ParserTests, wrong_introduction) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = ",whats up\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code,
              redis_async::error::make_error_code(redis_async::error::errc::wrong_introduction));
}

TEST(ParserTests, empty_buffer) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code,
              redis_async::error::make_error_code(redis_async::error::errc::not_enough_data));
}

TEST(ParserTests, simple_string_not_enough) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;
    auto expected_code =
        redis_async::error::make_error_code(redis_async::error::errc::not_enough_data);

    std::ostream(&buff) << "+Some string\n";

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, expected_code);
    buff.consume(buff.size());

    std::ostream(&buff) << "+Some string\r";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, expected_code);
    buff.consume(buff.size());

    std::ostream(&buff) << "+Some string";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, expected_code);
    buff.consume(buff.size());
}

TEST(ParserTests, error_not_enough) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;
    auto expected_code =
        redis_async::error::make_error_code(redis_async::error::errc::not_enough_data);

    std::ostream(&buff) << "-Some Error\n";

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, expected_code);
    buff.consume(buff.size());

    std::ostream(&buff) << "-Some Error\r";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, expected_code);
    buff.consume(buff.size());

    std::ostream(&buff) << "-Some Error";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, expected_code);
    buff.consume(buff.size());
}
TEST(ParserTests, int_wrong_data) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;
    auto not_enough =
        redis_async::error::make_error_code(redis_async::error::errc::not_enough_data);
    auto conv_error =
        redis_async::error::make_error_code(redis_async::error::errc::count_conversion);

    std::ostream(&buff) << ":123\n";

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << ":123\r";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << ":123";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << ":beef\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, conv_error);
    buff.consume(buff.size());

    std::ostream(&buff) << ":42357846237945625234652634523-05687359403\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, conv_error);
    buff.consume(buff.size());

    std::ostream(&buff) << ":4235784623794562523465263452305687359403\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, conv_error);
    buff.consume(buff.size());
}

TEST(ParserTests, bulk_string_wrong_data) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;
    auto not_enough =
        redis_async::error::make_error_code(redis_async::error::errc::not_enough_data);
    auto conv_error =
        redis_async::error::make_error_code(redis_async::error::errc::count_conversion);
    auto range_error =
        redis_async::error::make_error_code(redis_async::error::errc::count_range);
    auto term_error =
        redis_async::error::make_error_code(redis_async::error::errc::bulk_terminator);

    std::ostream(&buff) << "$123\n";

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << "$123\r";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << "$123";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << "$3\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << "$beef\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, conv_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "$42357846237945625234652634523-05687359403\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, conv_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "$4235784623794562523465263452305687359403\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, conv_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "$-100\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, range_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "$10\r\nSome string";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << "$10\r\nSome string\r";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, term_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "$10\r\nSome string\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, term_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "$10\r\nSome string\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, term_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "$10\r\nSome\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

}

TEST(ParserTests, array_wrong_data) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;
    auto not_enough =
        redis_async::error::make_error_code(redis_async::error::errc::not_enough_data);
    auto conv_error =
        redis_async::error::make_error_code(redis_async::error::errc::count_conversion);
    auto range_error =
        redis_async::error::make_error_code(redis_async::error::errc::count_range);
    auto term_error =
        redis_async::error::make_error_code(redis_async::error::errc::bulk_terminator);
    auto intr_error =
        redis_async::error::make_error_code(redis_async::error::errc::wrong_introduction);

    std::ostream(&buff) << "*123\n";

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    auto result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << "*123\r";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << "*123";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << "*3\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << "*beef\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, conv_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "*42357846237945625234652634523-05687359403\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, conv_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "*4235784623794562523465263452305687359403\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, conv_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "*-100\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, range_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "*10\r\nSome string";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, intr_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "*2\r\n$4\r\nSome\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, not_enough);
    buff.consume(buff.size());

    std::ostream(&buff) << "*2\r\n$4\r\nSome\r\n$-4\r\nSome";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, range_error);
    buff.consume(buff.size());

    std::ostream(&buff) << "*2\r\n+Some string\r\n&4\r\nSome\r\n";

    data = buff.data();
    parsed_result = redis_async::details::raw_parse(Iterator::begin(data), Iterator::end(data));
    result = boost::get<redis_async::details::protocol_error_t>(parsed_result);
    ASSERT_EQ(result.code, intr_error);
    buff.consume(buff.size());
}
