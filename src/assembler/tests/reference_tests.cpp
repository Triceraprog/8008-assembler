#include "evaluator.h"
#include "options.h"
#include "symbol_table.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(TrimmingString, trims_tabs) {
    std::string source{"\t\t100\t\t\t"};

    const auto & destination = trim_string(source);

    ASSERT_THAT(destination, Eq("100"));
}

TEST(TrimmingString, trims_nothing) {
    std::string source{"100"};

    const auto & destination = trim_string(source);

    ASSERT_THAT(destination, Eq("100"));
}


struct EvaluateArgumentFixture : public Test
{
    SymbolTable table;
    Options options;
};

TEST_F(EvaluateArgumentFixture, evaluates_int)
{
    auto value = evaluate_argument(options, table, 0, "1234");
    ASSERT_THAT(value, Eq(1234));
}

TEST_F(EvaluateArgumentFixture, evaluates_int_with_spaces)
{
    auto value = evaluate_argument(options, table, 0, "  1234  ");
    ASSERT_THAT(value, Eq(1234));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_with_suffix)
{
    auto value = evaluate_argument(options, table, 0, "100o");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_with_suffix_upper_case)
{
    auto value = evaluate_argument(options, table, 0, "100O");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_by_default)
{
    options.input_num_as_octal = true;
    auto value = evaluate_argument(options, table, 0, "100");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_by_default_with_tabs)
{
    options.input_num_as_octal = true;
    auto value = evaluate_argument(options, table, 0, "\t\t100\t\t\t");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_suffix)
{
    auto value = evaluate_argument(options, table, 0, "0FFh");
    ASSERT_THAT(value, Eq(255));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_suffix_upper_case)
{
    auto value = evaluate_argument(options, table, 0, "0FFH");
    ASSERT_THAT(value, Eq(255));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_prefix)
{
    auto value = evaluate_argument(options, table, 0, "0x1000");
    ASSERT_THAT(value, Eq(4096));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_prefix_upper_case)
{
    auto value = evaluate_argument(options, table, 0, "0X1000");
    ASSERT_THAT(value, Eq(4096));
}

TEST_F(EvaluateArgumentFixture, evaluates_binary_with_suffix)
{
    auto value = evaluate_argument(options, table, 0, "10000000b");
    ASSERT_THAT(value, Eq(128));
}

TEST_F(EvaluateArgumentFixture, evaluates_addition)
{
    auto value = evaluate_argument(options, table, 0, "2 + 0x2");
    ASSERT_THAT(value, Eq(4));
}

TEST_F(EvaluateArgumentFixture, evaluates_substraction)
{
    // TODO: Needs to trim the binary string values for it to work with a space after the minus sign
    auto value = evaluate_argument(options, table, 0, "02h -10b");
    ASSERT_THAT(value, Eq(0));
}

TEST_F(EvaluateArgumentFixture, evaluates_multiplication)
{
    auto value = evaluate_argument(options, table, 0, "100o*2");
    ASSERT_THAT(value, Eq(128));
}

TEST_F(EvaluateArgumentFixture, evaluates_division)
{
    auto value = evaluate_argument(options, table, 0, "100o/2");
    ASSERT_THAT(value, Eq(32));
}

TEST_F(EvaluateArgumentFixture, evaluates_symbol)
{
    table.define_symbol("TEST", 127);
    auto value = evaluate_argument(options, table, 0, "TEST");
    ASSERT_THAT(value, Eq(127));
}

TEST_F(EvaluateArgumentFixture, evaluates_FFh_as_a_symbol)
{
    table.define_symbol("FFh", 123);
    auto value = evaluate_argument(options, table, 0, "FFh");
    ASSERT_THAT(value, Eq(123));
}

TEST_F(EvaluateArgumentFixture, evaluates_high_low_byte)
{
    auto value = evaluate_argument(options, table, 0, "0xFF#1b");
    ASSERT_THAT(value, Eq(65281));
}

TEST_F(EvaluateArgumentFixture, evaluates_complex_expression)
{
    // NOTE: The expression is read from left to right, without arithmetic priorities
    auto value = evaluate_argument(options, table, 0, "2-4+10*2");
    ASSERT_THAT(value, Eq(16));
}

TEST_F(EvaluateArgumentFixture, evaluates_an_expression_with_a_symbol)
{
    table.define_symbol("TEST", 3);
    auto value = evaluate_argument(options, table, 0, "TEST*TEST");
    ASSERT_THAT(value, Eq(9));
}
