#include "evaluator.h"
#include "options.h"
#include "symbol_table.h"

#include "gmock/gmock.h"

using namespace testing;

struct EvaluateArgumentFixture : public Test
{
    SymbolTable table;
    Options options;
};

TEST_F(EvaluateArgumentFixture, evaluates_int)
{
    auto value = evaluate_argument(options, table, "1234");
    ASSERT_THAT(value, Eq(1234));
}

TEST_F(EvaluateArgumentFixture, evaluates_int_with_spaces)
{
    auto value = evaluate_argument(options, table, "  1234  ");
    ASSERT_THAT(value, Eq(1234));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_with_suffix)
{
    auto value = evaluate_argument(options, table, "100o");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_with_suffix_upper_case)
{
    auto value = evaluate_argument(options, table, "100O");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_by_default)
{
    options.input_num_as_octal = true;
    auto value = evaluate_argument(options, table, "100");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_by_default_with_tabs)
{
    options.input_num_as_octal = true;
    auto value = evaluate_argument(options, table, "\t\t100\t\t\t");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_suffix)
{
    auto value = evaluate_argument(options, table, "0FFh");
    ASSERT_THAT(value, Eq(255));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_suffix_upper_case)
{
    auto value = evaluate_argument(options, table, "0FFH");
    ASSERT_THAT(value, Eq(255));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_prefix)
{
    auto value = evaluate_argument(options, table, "0x1000");
    ASSERT_THAT(value, Eq(4096));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_prefix_upper_case)
{
    auto value = evaluate_argument(options, table, "0X1000");
    ASSERT_THAT(value, Eq(4096));
}

TEST_F(EvaluateArgumentFixture, evaluates_binary_with_suffix)
{
    auto value = evaluate_argument(options, table, "10000000b");
    ASSERT_THAT(value, Eq(128));
}

TEST_F(EvaluateArgumentFixture, evaluates_addition)
{
    auto value = evaluate_argument(options, table, "2 + 0x2");
    ASSERT_THAT(value, Eq(4));
}

TEST_F(EvaluateArgumentFixture, evaluates_substraction)
{
    auto value = evaluate_argument(options, table, "02h - 10b");
    ASSERT_THAT(value, Eq(0));
}

TEST_F(EvaluateArgumentFixture, evaluates_multiplication)
{
    auto value = evaluate_argument(options, table, "100o*2");
    ASSERT_THAT(value, Eq(128));
}

TEST_F(EvaluateArgumentFixture, evaluates_division)
{
    auto value = evaluate_argument(options, table, "100o/2");
    ASSERT_THAT(value, Eq(32));
}

TEST_F(EvaluateArgumentFixture, evaluates_symbol)
{
    table.define_symbol("TEST", 127);
    auto value = evaluate_argument(options, table, "TEST");
    ASSERT_THAT(value, Eq(127));
}

TEST_F(EvaluateArgumentFixture, evaluates_FFh_as_a_symbol)
{
    table.define_symbol("FFh", 123);
    auto value = evaluate_argument(options, table, "FFh");
    ASSERT_THAT(value, Eq(123));
}

TEST_F(EvaluateArgumentFixture, evaluates_high_low_byte)
{
    auto value = evaluate_argument(options, table, "0xFF#1b");
    ASSERT_THAT(value, Eq(65281));
}

TEST_F(EvaluateArgumentFixture, evaluates_complex_expression)
{
    // NOTE: The expression is read from left to right, without arithmetic priorities
    auto value = evaluate_argument(options, table, "2-4+10*2");
    ASSERT_THAT(value, Eq(16));
}

TEST_F(EvaluateArgumentFixture, evaluates_throws_an_exception_if_the_symbol_is_not_found)
{
    ASSERT_THROW(evaluate_argument(options, table, "TEST"), CannotFindSymbol);
}

TEST_F(EvaluateArgumentFixture, throws_if_expression_is_ill_formed)
{
    ASSERT_THROW(evaluate_argument(options, table, "TEST*"), IllFormedExpression);
}

TEST_F(EvaluateArgumentFixture, throws_if_unknown_operation)
{
    ASSERT_THROW(evaluate_argument(options, table, "TEST.TEST"), UnknownOperation);
}

TEST_F(EvaluateArgumentFixture, throws_if_invalid_number)
{
    ASSERT_THROW(evaluate_argument(options, table, "2*0xfffffffffffffff"), InvalidNumber);
}

TEST_F(EvaluateArgumentFixture, throws_if_no_expression)
{
    ASSERT_THROW(evaluate_argument(options, table, "+"), ExpectedValue);
}
