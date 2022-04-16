#include "context_stack.h"

#include "context.h"

ContextStack::ContextStack(const Options& options)
{
    stack.push(std::make_shared<Context>(options));
}

std::shared_ptr<Context> ContextStack::get_current_context() { return stack.top(); }
std::shared_ptr<Context> ContextStack::get_current_context() const { return stack.top(); }

void ContextStack::push() { stack.push(std::make_shared<Context>(stack.top())); }
void ContextStack::pop()
{
    // Always keep the top level.
    if (stack.size() >= 2)
    {
        stack.pop();
    }
}