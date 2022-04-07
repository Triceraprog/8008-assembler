#ifndef INC_8008_ASSEMBLER_CONTEXT_H
#define INC_8008_ASSEMBLER_CONTEXT_H

#include "symbol_table.h"

#include <stack>

class Options;

struct Context
{
    explicit Context(const Options& options);

    Options & get_options();
    const Options & get_options() const;

    void define_symbol(std::string_view symbol_name, int value);
    [[nodiscard]] std::tuple<bool, int> get_symbol_value(std::string_view symbol_name) const;
    void list_symbols(std::ostream& output);

    void push();
    void pop();

private:
    std::stack<Options> option_stack;
    SymbolTable symbol_table;
};

#endif //INC_8008_ASSEMBLER_CONTEXT_H
