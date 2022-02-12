#include "symbol_table.h"

#include <iomanip>
#include <iostream>

void SymbolTable::define_symbol(const std::string_view symbol_name, int value)
{
    std::string upper{symbol_name};
    transform(upper.begin(), upper.end(), upper.begin(), toupper);

    // Symbols keys are upper string labels
    symbols[upper] = value;

    // The insertion order also keeps the original casing.
    insertion_order.emplace_back(symbol_name);
}

std::tuple<bool, int> SymbolTable::get_symbol_value(const std::string_view symbol_name) const
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

void SymbolTable::list_symbols(std::ostream& output)
{
    output << "Symbol Count: " << symbols.size() << "\n";
    output << "    Symbol  Oct Val  DecVal\n";
    output << "    ------  -------  ------\n";
    for (auto& sorted_label : insertion_order)
    {
        std::string upper{sorted_label};
        transform(upper.begin(), upper.end(), upper.begin(), toupper);

        auto& [label, value] = *symbols.find(upper);
        if (value > 255)
        {
            const auto high = (value >> 8) & 0xFF;
            const auto low = value & 0xFF;
            output << std::setw(10) << sorted_label.c_str() << "   ";
            output << std::setw(2) << std::oct << high << " ";
            output << std::setw(3) << std::setfill('0') << std::oct << low << "  ";
            output << std::setw(5) << std::setfill(' ') << std::dec << value;
            output << "\n";
        }
        else
        {
            output << std::setw(10) << sorted_label.c_str() << "      ";
            output << std::setw(3) << std::setfill('0') << std::oct << value << "  ";
            output << std::setw(5) << std::setfill(' ') << std::dec << value;
            output << "\n";
        }
    }
}
