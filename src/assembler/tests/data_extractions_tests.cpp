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
    int out_data[20];
    auto number = decode_data(options, table, 0, "DATA 100", out_data);

    ASSERT_THAT(number, Eq(1));
    ASSERT_THAT(out_data[0], Eq(100));
}

TEST_F(DataExtractorFixture, throws_if_too_much_data)
{
    int out_data[20];
    ASSERT_THROW(decode_data(options, table, 0, "DATA 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13",
                             out_data),
                 DataTooLong);
}
