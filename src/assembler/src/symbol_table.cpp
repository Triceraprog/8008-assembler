#include "symbol_table.h"
#include <algorithm>

void SymbolTable::define_symbol(const char* symbol_name, int value)
{
    std::string upper{symbol_name};
    transform(upper.begin(), upper.end(), upper.begin(), toupper);

    // Symbols keys are upper string labels
    symbols[upper] = value;

    // The insertion order also keeps the original casing.
    insertion_order.emplace_back(symbol_name);
}

std::tuple<bool, int> SymbolTable::get_symbol_value(const char* symbol_name) const
{
    std::string upper{symbol_name};
    transform(upper.begin(), upper.end(), upper.begin(), toupper);

    auto it = symbols.find(std::string{upper});
    if (it != symbols.end())
    {
        return {true, it->second};
    }
    return {false, 0};
}

void SymbolTable::list_symbols(FILE* lfp)
{
    fprintf(lfp, "Symbol Count: %lu\n", symbols.size());
    fprintf(lfp, "    Symbol  Oct Val  DecVal\n");
    fprintf(lfp, "    ------  -------  ------\n");
    for (auto& sorted_label : insertion_order)
    {
        std::string upper{sorted_label};
        transform(upper.begin(), upper.end(), upper.begin(), toupper);

        auto& [label, value] = *symbols.find(upper);
        if (value > 255)
        {
            const auto high = (value >> 8) & 0xFF;
            const auto low = value & 0xFF;
            fprintf(lfp, "%10s   %2o %03o  %5d\n", sorted_label.c_str(), high, low, value);
        }
        else
        {
            fprintf(lfp, "%10s      %03o  %5d\n", sorted_label.c_str(), value, value);
        }
    }
}
