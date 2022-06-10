#include "evaluator.h"
#include "context.h"
#include "legacy_evaluate.h"
#include "options.h"

#include <iostream>

int evaluate_argument(const Context& context, std::string_view arg)
{
    if (arg.starts_with("\\HB\\"))
    {
        int value = evaluate_argument(context, arg.begin() + 4);
        return ((value >> 8) & 0xFF);
    }
    if (arg.starts_with("H(") && arg.ends_with(')'))
    {
        int value = evaluate_argument(context, arg.substr(2, arg.size() - 2 - 1));
        return ((value >> 8) & 0xFF);
    }

    if (arg.starts_with("\\LB\\"))
    {
        int value = evaluate_argument(context, arg.begin() + 4);
        return (value & 0xFF);
    }
    if (arg.starts_with("L(") && arg.ends_with(')'))
    {
        int value = evaluate_argument(context, arg.substr(2, arg.size() - 2 - 1));
        return (value & 0xFF);
    }

    if (context.get_options().debug)
    {
        std::cout << "evaluating " << arg << "\n";
    }

    int result = legacy_evaluator(context, arg);

    if (context.get_options().debug)
    {
        std::cout << "     got final value " << result << "\n";
    }

    return result;
}
