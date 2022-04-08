#include "context_stack.h"

#include "context.h"
#include "options.h"
#include "symbol_table.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(ContextStackStack, starts_with_a_context)
{
    Options options;
    ContextStack ctx_stack(options);

    auto context = ctx_stack.get_current_context();

    ASSERT_THAT(context, NotNull());
}

TEST(ContextStack, keeps_options_when_pushing_the_context)
{
    Options options;
    options.new_syntax = true;

    ContextStack ctx_stack(options);

    ASSERT_THAT(ctx_stack.get_current_context()->get_options().new_syntax, IsTrue());
    ctx_stack.push();
    ASSERT_THAT(ctx_stack.get_current_context()->get_options().new_syntax, IsTrue());
}

TEST(ContextStack, can_change_new_context_options_and_pop_gets_old_value)
{
    Options options;
    options.new_syntax = true;

    ContextStack ctx_stack(options);
    ctx_stack.push();
    ctx_stack.get_current_context()->get_options().new_syntax = false;

    ASSERT_THAT(ctx_stack.get_current_context()->get_options().new_syntax, IsFalse());
    ctx_stack.pop();
    ASSERT_THAT(ctx_stack.get_current_context()->get_options().new_syntax, IsTrue());
}
