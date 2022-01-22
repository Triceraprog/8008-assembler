#include "symbol_table.h"

#include "gmock/gmock.h"

using namespace testing;

extern int evaluate_argument(const SymbolTable& symbol_table, int current_line_count,
                             std::string_view arg);

TEST(EvaluateArguement, decodes_int)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "1234");

    ASSERT_THAT(value, Eq(1234));
}
