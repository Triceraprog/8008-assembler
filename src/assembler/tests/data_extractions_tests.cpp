#include "data_extraction.h"
#include "options.h"
#include "symbol_table.h"

#include "gmock/gmock.h"

using namespace testing;

struct DataExtractorFixture : public Test
{
    SymbolTable table;
    Options options;
};

TEST_F(DataExtractorFixture, evaluates_int)
{
    std::vector<int> out_data;
    auto number = decode_data(options, table, "DATA 100", out_data);

    ASSERT_THAT(number, Eq(1));
    ASSERT_THAT(out_data[0], Eq(100));
}

TEST_F(DataExtractorFixture, throws_if_too_much_data)
{
    std::vector<int> out_data;
    ASSERT_THROW(
            decode_data(options, table, "DATA 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13", out_data),
            DataTooLong);
}

TEST_F(DataExtractorFixture, throws_if_finds_an_unknown_escape_char)
{
    std::vector<int> out_data;
    ASSERT_THROW(decode_data(options, table, "DATA \"\\u0001\"", out_data), UnknownEscapeSequence);
}
