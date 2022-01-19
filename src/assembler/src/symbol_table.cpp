#include "symbol_table.h"

void SymbolTable::define_symbol(const char* symbol_name, int value)
{ symbols[symbol_name] = value; }

std::tuple<bool, int> SymbolTable::get_symbol_value(const char* symbol) const
{
    auto it = symbols.find(std::string{symbol});
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
    for (auto& [label, value] : symbols)
    {
        if (value > 255)
        {
            const auto high = (value >> 8) & 0xFF;
            const auto low = value & 0xFF;
            fprintf(lfp, "%10s   %2o %03o  %5d\n", label.c_str(), high, low, value);
        }
        else
        {
            fprintf(lfp, "%10s      %03o  %5d\n", label.c_str(), value, value);
        }
    }
}
