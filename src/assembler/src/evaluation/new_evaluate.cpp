#include "new_evaluate.h"

namespace SE = SimpleEvaluator;

struct Configuration {
    std::unordered_map<std::string, std::int32_t> symbols{
            {"eight", 8},
            {"two", 2},
            {"one", 1}};

    int symbol_to_value(const std::string& symbol_name) const
    {
        auto it = symbols.find(symbol_name);
        return (it == std::end(symbols)) ? 0 : it->second;
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

int new_evaluator(const Context& context, std::string_view arg) { return 0; }
