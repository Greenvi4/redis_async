//
// Created by niko on 02.07.2021.
//
#include <redis_async/details/protocol/message.hpp>
#include <redis_async/details/protocol/parser.hpp>

#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/streambuf.hpp>

#include <gtest/gtest.h>

TEST(ParserTests, raw_cmd) {
    using redis_async::details::command_container_t;
    using redis_async::details::message;
    using redis_async::details::single_command_t;
    using Buffer = message::buffer_type;

    {
        single_command_t ping{"PING", "Hello, World!"};
        message m(ping);
        const std::string expected = "*2\r\n$4\r\nPING\r\n$13\r\nHello, World!\r\n";
        Buffer result(m.buffer().first, m.buffer().second);
        ASSERT_EQ(std::string(result.begin(), result.end()), expected);
    }
    {
        single_command_t cmd("HSET", "key", "value1", "", "value2", "");
        message m(cmd);
        const std::string expected("*6\r\n$4\r\nHSET\r\n$3\r\nkey\r\n$6\r\nvalue1\r\n$0\r\n\r\n$"
                                   "6\r\nvalue2\r\n$0\r\n\r\n");
        Buffer result(m.buffer().first, m.buffer().second);
        ASSERT_EQ(std::string(result.begin(), result.end()), expected);
    }
    {
        command_container_t cont = {{"PING", "Hello, World!"}, {"LPUSH", "list", "value"}};
        message m(cont);
        const std::string expected = "*2\r\n$4\r\nPING\r\n$13\r\nHello, World!\r\n"
                                     "*3\r\n$5\r\nLPUSH\r\n$4\r\nlist\r\n$5\r\nvalue\r\n";
        Buffer result(m.buffer().first, m.buffer().second);
        ASSERT_EQ(std::string(result.begin(), result.end()), expected);
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
        redis_async::details::raw_parse(Iterator ::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    ASSERT_EQ("OK", boost::get<redis_async::string_t>(positive_parse_result.result).str);

    buff.consume(positive_parse_result.consumed);
}

TEST(ParserTests, error) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "-Some Error\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator ::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    ASSERT_EQ("Some Error", boost::get<redis_async::error_t>(positive_parse_result.result).str);

    buff.consume(positive_parse_result.consumed);
}

TEST(ParserTests, integer) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = ":-555423\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator ::begin(data), Iterator::end(data));
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
        redis_async::details::raw_parse(Iterator ::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    ASSERT_EQ("some", boost::get<redis_async::string_t>(positive_parse_result.result).str);
}

TEST(ParserTests, bulk_string_emply) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "$0\r\n\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator ::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    ASSERT_EQ("", boost::get<redis_async::string_t>(positive_parse_result.result).str);
}

TEST(ParserTests, nil) {
    using Buffer = boost::asio::streambuf;
    using Iterator = boost::asio::buffers_iterator<Buffer::const_buffers_type, char>;
    Buffer buff;

    std::string answer = "$-1\r\n";
    std::ostream(&buff) << answer;

    auto data = buff.data();
    auto parsed_result =
        redis_async::details::raw_parse(Iterator ::begin(data), Iterator::end(data));
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
        redis_async::details::raw_parse(Iterator ::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    auto array = boost::get<redis_async::array_holder_t>(positive_parse_result.result);
    ASSERT_EQ(array.elements.size(), 2);
    ASSERT_EQ("some", boost::get<redis_async::string_t>(array.elements[0]).str);
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
        redis_async::details::raw_parse(Iterator ::begin(data), Iterator::end(data));
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
        redis_async::details::raw_parse(Iterator ::begin(data), Iterator::end(data));
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
        redis_async::details::raw_parse(Iterator ::begin(data), Iterator::end(data));
    auto positive_parse_result =
        boost::get<redis_async::details::positive_parse_result_t>(parsed_result);

    ASSERT_EQ(answer.size(), positive_parse_result.consumed);
    auto array = boost::get<redis_async::array_holder_t>(positive_parse_result.result);
    ASSERT_EQ(array.elements.size(), 3);
    ASSERT_EQ("some", boost::get<redis_async::string_t>(array.elements[0]).str);
    ASSERT_NO_THROW(boost::get<redis_async::nil_t>(array.elements[1]));
    ASSERT_EQ(-555423, boost::get<redis_async::int_t>(array.elements[2]));
}
