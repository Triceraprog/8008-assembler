#include "evaluator.h"

#include "options.h"

#include "gmock/gmock.h"

using namespace testing;

struct EvaluateArgumentFixture : public Test
{
    Options options;
    Context context{options};
};

TEST_F(EvaluateArgumentFixture, evaluates_int)
{
    auto value = evaluate_argument(context, "1234");
    ASSERT_THAT(value, Eq(1234));
}

TEST_F(EvaluateArgumentFixture, evaluates_int_with_spaces)
{
    auto value = evaluate_argument(context, "  1234  ");
    ASSERT_THAT(value, Eq(1234));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_with_suffix)
{
    auto value = evaluate_argument(context, "100o");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_with_suffix_upper_case)
{
    auto value = evaluate_argument(context, "100O");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_by_default)
{
    context.get_options().input_num_as_octal = true;
    auto value = evaluate_argument(context, "100");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_octal_by_default_with_tabs)
{
    context.get_options().input_num_as_octal = true;
    auto value = evaluate_argument(context, "\t\t100\t\t\t");
    ASSERT_THAT(value, Eq(64));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_suffix)
{
    auto value = evaluate_argument(context, "0FFh");
    ASSERT_THAT(value, Eq(255));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_suffix_upper_case)
{
    auto value = evaluate_argument(context, "0FFH");
    ASSERT_THAT(value, Eq(255));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_prefix)
{
    auto value = evaluate_argument(context, "0x1000");
    ASSERT_THAT(value, Eq(4096));
}

TEST_F(EvaluateArgumentFixture, evaluates_hex_with_prefix_upper_case)
{
    auto value = evaluate_argument(context, "0X1000");
    ASSERT_THAT(value, Eq(4096));
}

TEST_F(EvaluateArgumentFixture, evaluates_binary_with_suffix)
{
    auto value = evaluate_argument(context, "10000000b");
    ASSERT_THAT(value, Eq(128));
}

TEST_F(EvaluateArgumentFixture, evaluates_char)
{
    auto value = evaluate_argument(context, "' '");
    ASSERT_THAT(value, Eq(32));
}

TEST_F(EvaluateArgumentFixture, evaluates_addition)
{
    auto value = evaluate_argument(context, "2 + 0x2");
    ASSERT_THAT(value, Eq(4));
}

TEST_F(EvaluateArgumentFixture, evaluates_substraction)
{
    auto value = evaluate_argument(context, "02h - 10b");
    ASSERT_THAT(value, Eq(0));
}

TEST_F(EvaluateArgumentFixture, evaluates_multiplication)
{
    auto value = evaluate_argument(context, "100o*2");
    ASSERT_THAT(value, Eq(128));
}

TEST_F(EvaluateArgumentFixture, evaluates_division)
{
    auto value = evaluate_argument(context, "100o/2");
    ASSERT_THAT(value, Eq(32));
}

TEST_F(EvaluateArgumentFixture, evaluates_symbol)
{
    context.define_symbol("TEST", 127);
    auto value = evaluate_argument(context, "TEST");
    ASSERT_THAT(value, Eq(127));
}

TEST_F(EvaluateArgumentFixture, evaluates_FFh_as_a_symbol)
{
    context.define_symbol("FFh", 123);
    auto value = evaluate_argument(context, "FFh");
    ASSERT_THAT(value, Eq(123));
}

TEST_F(EvaluateArgumentFixture, evaluates_high_low_byte)
{
    auto value = evaluate_argument(context, "0xFF#1b");
    ASSERT_THAT(value, Eq(65281));
}

TEST_F(EvaluateArgumentFixture, evaluates_complex_expression)
{
    // NOTE: The expression is read from left to right, without arithmetic priorities
    auto value = evaluate_argument(context, "2-4+10*2");
    ASSERT_THAT(value, Eq(16));
}

TEST_F(EvaluateArgumentFixture, evaluates_throws_an_exception_if_the_symbol_is_not_found)
{
    ASSERT_THROW(evaluate_argument(context, "TEST"), CannotFindSymbol);
}

TEST_F(EvaluateArgumentFixture, throws_if_expression_is_ill_formed)
{
    ASSERT_THROW(evaluate_argument(context, "TEST*"), IllFormedExpression);
}

TEST_F(EvaluateArgumentFixture, throws_if_unknown_operation)
{
    ASSERT_THROW(evaluate_argument(context, "TEST.TEST"), UnknownOperation);
}

TEST_F(EvaluateArgumentFixture, throws_if_invalid_number)
{
    ASSERT_THROW(evaluate_argument(context, "2*0xfffffffffffffff"), InvalidNumber);
}

TEST_F(EvaluateArgumentFixture, throws_if_no_expression)
{
    ASSERT_THROW(evaluate_argument(context, "+"), ExpectedValue);
}
