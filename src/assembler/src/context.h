#ifndef INC_8008_ASSEMBLER_CONTEXT_H
#define INC_8008_ASSEMBLER_CONTEXT_H

#include "options.h"
#include "symbol_table.h"

#include <memory>
#include <ostream>
#include <string_view>

struct Context
{
    explicit Context(Options options);
    explicit Context(const std::shared_ptr<Context>& other_context);
    Context(Context&) = delete;

    Options& get_options();
    [[nodiscard]] const Options& get_options() const;

    void define_symbol(std::string_view symbol_name, int value);
    [[nodiscard]] std::tuple<bool, int> get_symbol_value(std::string_view symbol_name) const;
    void list_symbols(std::ostream& output);

private:
    Options options;
    SymbolTable symbol_table;

    const std::shared_ptr<Context> parent;
};

#endif //INC_8008_ASSEMBLER_CONTEXT_H
