#ifndef INC_8008_ASSEMBLER_CONTEXT_H
#define INC_8008_ASSEMBLER_CONTEXT_H

#include "options.h"
#include "symbol_table.h"

struct Context
{
    explicit Context(const Options& options);

    void define_symbol(std::string_view symbol_name, int value);
    [[nodiscard]] std::tuple<bool, int> get_symbol_value(std::string_view symbol_name) const;
    void list_symbols(std::ostream& output);

    Options options;

private:
    SymbolTable symbol_table;
};

#endif //INC_8008_ASSEMBLER_CONTEXT_H
