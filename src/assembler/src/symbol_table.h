#ifndef INC_8008_ASSEMBLER_SYMBOL_TABLE_H
#define INC_8008_ASSEMBLER_SYMBOL_TABLE_H

#include <unordered_map>
#include <cstdio>
#include <string>
#include <vector>

class SymbolTable
{
public:
    void define_symbol(const char* symbol_name, int value);
    std::tuple<bool, int> get_symbol_value(const char* symbol) const;
    void list_symbols(FILE* lfp);

private:
    std::unordered_map<std::string, int> symbols;
    std::vector<std::string> insertion_order;
};

#endif //INC_8008_ASSEMBLER_SYMBOL_TABLE_H
