#include "new_evaluate.h"
#include "context.h"
#include "evaluate.h"

namespace SE = SimpleEvaluator;

struct Configuration
{
    Context* context{};

    int symbol_to_value(const std::string& symbol_name) const
    {
        const auto [success, value] = context->get_symbol_value(symbol_name);
        if (success)
        {
            return value;
        }
        throw CannotFindSymbol{symbol_name};
    }

    std::unordered_map<std::string, SE::function_type> functions{
            {"square", [](const int* data) { return data[0] * data[0]; }},
    };

    SE::function_type function_to_value(const std::string& function_name) const
    {
        auto it = functions.find(function_name);
        return (it == std::end(functions)) ? [](const int*) { return 0; } : it->second;
    }
};

int new_evaluator(const Context& context, std::string_view arg)
{
    static Configuration configuration;

    // The glue is not pretty...
    configuration.context = const_cast<Context*>(&context);
    auto value = SE::evaluate(configuration,
                              EvaluationFlags::get_flags_from_options(context.get_options()), arg);

    configuration.context = nullptr;
    return value;
}
