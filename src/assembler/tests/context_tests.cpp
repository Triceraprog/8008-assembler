#include "context.h"

#include "options.h"
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

TEST(Context, keeps_options_when_pushing_the_context)
{
    Options options;

    options.new_syntax = true;
    Context ctx(options);
    ctx.push();

    ASSERT_THAT(ctx.get_options().new_syntax, IsTrue());
}

TEST(Context, can_change_new_context_options_and_pop_gets_old_value)
{
    Options options;
    options.new_syntax = true;

    Context ctx(options);
    ctx.push();
    ctx.get_options().new_syntax = false;

    ASSERT_THAT(ctx.get_options().new_syntax, IsFalse());

    ctx.pop();

    ASSERT_THAT(ctx.get_options().new_syntax, IsTrue());
}
