#ifndef INC_8008_ASSEMBLER_SYMBOL_TABLE_H
#define INC_8008_ASSEMBLER_SYMBOL_TABLE_H

#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

class SymbolTable
{
public:
    void define_symbol(std::string_view symbol_name, int value);
    std::tuple<bool, int> get_symbol_value(std::string_view symbol_name) const;
    void list_symbols(FILE* lfp);

private:
    std::unordered_map<std::string, int> symbols;
    std::vector<std::string> insertion_order;
};

#endif //INC_8008_ASSEMBLER_SYMBOL_TABLE_H
