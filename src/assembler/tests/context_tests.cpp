#include "context.h"

#include "macro_content.h"
#include "options.h"

#include <memory>

#include "gmock/gmock.h"

using namespace testing;

TEST(Context, needs_options)
{
    Options options;

    options.new_syntax = true;
    Context ctx(options);

    ASSERT_THAT(options.new_syntax, IsTrue());
}

TEST(Context, can_define_a_symbol_associated_to_int_value)
{
    Options options;
    Context ctx(options);

    ctx.define_symbol("TEST", 123);

    auto [success, value] = ctx.get_symbol_value("TEST");
    ASSERT_THAT(success, IsTrue());
    ASSERT_THAT(value, Eq(123));

    auto [failure, no_value] = ctx.get_symbol_value("NO");
    ASSERT_THAT(failure, IsFalse());
}

TEST(Context, context_can_be_copy_constructed)
{
    Options options;
    options.new_syntax = true;

    auto ctx_1 = std::make_shared<Context>(options);
    Context ctx_2(ctx_1);

    ASSERT_THAT(ctx_2.get_options().new_syntax, IsTrue());
}

TEST(Context, context_can_be_copy_constructed_and_has_local_symbol_definitions)
{
    Options options;
    auto ctx_1 = std::make_shared<Context>(options);
    Context ctx_2(ctx_1);

    ctx_1->define_symbol("TEST", 123);
    ctx_2.define_symbol("TEST_2", 321);

    auto [success, value] = ctx_2.get_symbol_value("TEST");
    ASSERT_THAT(success, IsTrue());
    ASSERT_THAT(value, Eq(123));

    auto [success_2, value_2] = ctx_2.get_symbol_value("TEST_2");
    ASSERT_THAT(success_2, IsTrue());
    ASSERT_THAT(value_2, Eq(321));

    auto [failure, no_value] = ctx_2.get_symbol_value("NO");
    ASSERT_THAT(failure, IsFalse());

    auto [failure_2, no_value_2] = ctx_1->get_symbol_value("TEST_1");
    ASSERT_THAT(failure, IsFalse());
}

TEST(Context, can_receive_a_macro_content)
{
    Options options;
    Context ctx(options);

    auto macro_content = std::make_unique<MacroContent>("name", MacroContent::Parameters{});
    ctx.declare_macro(std::move(macro_content));
}

TEST(Context, cannot_declare_twice_a_macro_with_the_same_name)
{
    Options options;
    Context ctx(options);

    {
        auto macro_content = std::make_unique<MacroContent>("name", MacroContent::Parameters{});
        ctx.declare_macro(std::move(macro_content));
    }

    {
        auto second_macro_content =
                std::make_unique<MacroContent>("name", MacroContent::Parameters{});
        ASSERT_THROW(ctx.declare_macro(std::move(second_macro_content)), AlreadyDefinedMacro);
    }
}

TEST(Context, can_check_if_it_has_a_macro_by_name)
{
    Options options;
    Context ctx(options);

    ASSERT_THAT(ctx.has_macro("name"), IsFalse());
}
