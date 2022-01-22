#include "symbol_table.h"

#include "gmock/gmock.h"

using namespace testing;

extern int evaluate_argument(const SymbolTable& symbol_table, int current_line_count,
                             std::string_view arg);

// TODO: missing test, octal by default. Needs to pass the option for this.

TEST(EvaluateArguement, evaluates_int)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "1234");

    ASSERT_THAT(value, Eq(1234));
}

TEST(EvaluateArguement, evaluates_int_with_spaces)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "  1234  ");

    ASSERT_THAT(value, Eq(1234));
}

TEST(EvaluateArguement, evaluates_octal_with_suffix)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "100o");

    ASSERT_THAT(value, Eq(64));
}

TEST(EvaluateArguement, evaluates_octal_with_suffix_upper_case)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "100o");

    ASSERT_THAT(value, Eq(64));
}

TEST(EvaluateArguement, evaluates_hex_with_suffix)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "0FFh");

    ASSERT_THAT(value, Eq(255));
}

TEST(EvaluateArguement, evaluates_hex_with_suffix_upper_case)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "0FFH");

    ASSERT_THAT(value, Eq(255));
}

TEST(EvaluateArguement, evaluates_hex_with_prefix)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "0x1000");

    ASSERT_THAT(value, Eq(4096));
}

TEST(EvaluateArguement, evaluates_hex_with_prefix_upper_case)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "0X1000");

    ASSERT_THAT(value, Eq(4096));
}

TEST(EvaluateArguement, evaluates_binary_with_suffix)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "10000000b");

    ASSERT_THAT(value, Eq(128));
}

TEST(EvaluateArguement, evaluates_addition)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "2 + 0x2");

    ASSERT_THAT(value, Eq(4));
}

TEST(EvaluateArguement, evaluates_substraction)
{
    SymbolTable table;
    // TODO: Needs to trim the binary string values for it to work with a space after the minus sign
    auto value = evaluate_argument(table, 0, "02h -10b");

    ASSERT_THAT(value, Eq(0));
}

TEST(EvaluateArguement, evaluates_multiplication)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "100o*2");

    ASSERT_THAT(value, Eq(128));
}

TEST(EvaluateArguement, evaluates_division)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "100o/2");

    ASSERT_THAT(value, Eq(32));
}

TEST(EvaluateArguement, evaluates_symbol)
{
    SymbolTable table;
    table.define_symbol("TEST", 127);
    auto value = evaluate_argument(table, 0, "TEST");

    ASSERT_THAT(value, Eq(127));
}

TEST(EvaluateArguement, evaluates_FFh_as_a_symbol)
{
    SymbolTable table;
    table.define_symbol("FFh", 123);
    auto value = evaluate_argument(table, 0, "FFh");

    ASSERT_THAT(value, Eq(123));
}

TEST(EvaluateArguement, evaluates_high_low_byte)
{
    SymbolTable table;
    auto value = evaluate_argument(table, 0, "0xFF#1b");

    ASSERT_THAT(value, Eq(65281));
}

TEST(EvaluateArguement, evaluates_complex_expression)
{
    SymbolTable table;

    // NOTE: The expression is read from left to right, without arithmetic priorities
    auto value = evaluate_argument(table, 0, "2-4+10*2");

    ASSERT_THAT(value, Eq(16));
}

TEST(EvaluateArguement, evaluates_an_expression_with_a_symbol)
{
    SymbolTable table;
    table.define_symbol("TEST", 3);

    auto value = evaluate_argument(table, 0, "TEST*TEST");

    ASSERT_THAT(value, Eq(9));
}
