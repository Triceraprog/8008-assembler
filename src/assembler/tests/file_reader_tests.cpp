#include "gmock/gmock.h"

#include "files/file_reader.h"

#include <deque>

using namespace testing;

TEST(FileReader, is_empty_when_constructed)
{
    FileReader file_reader;

    ASSERT_THAT(std::begin(file_reader), Eq(std::end(file_reader)));
}

TEST(FileReader, is_not_empty_after_adding_an_iterator)
{
    const std::string input_value{"\n"};
    auto content = std::make_unique<std::istringstream>(input_value);

    FileReader file_reader;
    file_reader.append(std::move(content));

    ASSERT_THAT(std::begin(file_reader), Not(Eq(std::end(file_reader))));
}

TEST(FileReader, can_consume_one_line_and_reaches_end)
{
    const std::string input_value{"a"};
    auto content = std::make_unique<std::istringstream>(input_value);

    FileReader file_reader;
    file_reader.append(std::move(content));

    auto it = std::begin(file_reader);
    auto read_content = *it;
    ++it;

    ASSERT_THAT(it, Eq(std::end(file_reader)));
    ASSERT_THAT(read_content, Eq("a"));
}

TEST(FileReader, iterates_on_lines)
{
    const std::string input_value{"first line\nsecond line\n"};
    auto content = std::make_unique<std::istringstream>(input_value);

    FileReader file_reader;
    file_reader.append(std::move(content));

    std::vector<std::string> all_lines;
    std::copy(std::begin(file_reader), std::end(file_reader), std::back_inserter(all_lines));

    ASSERT_THAT(all_lines.size(), Eq(2));
}

TEST(FileReader, chains_two_input_streams)
{
    const std::string input_value_1{"first line\nsecond line"};
    const std::string input_value_2{"third line\nfourth line"};

    auto content_1 = std::make_unique<std::istringstream>(input_value_1);
    auto content_2 = std::make_unique<std::istringstream>(input_value_2);

    FileReader file_reader;
    file_reader.append(std::move(content_1));
    file_reader.append(std::move(content_2));

    std::vector<std::string> all_lines;
    std::copy(std::begin(file_reader), std::end(file_reader), std::back_inserter(all_lines));

    ASSERT_THAT(all_lines.size(), Eq(4));
}

TEST(FileReader, chains_three_input_streams_and_ignores_empty_one)
{
    const std::string input_value_1{"first line\nsecond line"};
    const std::string input_value_3{"third line\nfourth line"};

    auto content_1 = std::make_unique<std::istringstream>(input_value_1);
    auto content_2 = std::make_unique<std::istringstream>("");
    auto content_3 = std::make_unique<std::istringstream>(input_value_3);

    FileReader file_reader;
    file_reader.append(std::move(content_1));
    file_reader.append(std::move(content_2));
    file_reader.append(std::move(content_3));

    std::vector<std::string> all_lines;
    std::copy(std::begin(file_reader), std::end(file_reader), std::back_inserter(all_lines));

    ASSERT_THAT(all_lines.size(), Eq(4));
}

TEST(FileReader, input_stream_interrupted_by_another)
{
    const std::string input_value_1{"first line\nsecond line"};
    const std::string input_value_2{"interruption\nsecond interruption"};

    auto content_1 = std::make_unique<std::istringstream>(input_value_1);
    auto content_2 = std::make_unique<std::istringstream>(input_value_2);

    FileReader file_reader;
    file_reader.append(std::move(content_1));

    auto it = std::begin(file_reader);

    ASSERT_THAT(*it, Eq("first line"));

    file_reader.insert_now(std::move(content_2));

    ASSERT_THAT(*it, Eq("first line"));
    ++it;
    ASSERT_THAT(*it, Eq("interruption"));
    ++it;
    ASSERT_THAT(*it, Eq("second interruption"));
    ++it;
    ASSERT_THAT(*it, Eq("second line"));
    ++it;
    ASSERT_THAT(it, Eq(std::end(file_reader)));
}