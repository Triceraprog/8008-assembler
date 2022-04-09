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

    enum ParsingMode
    {
        ACTIVE,
        CONDITIONAL_TRUE,
        CONDITIONAL_FALSE,
    };

    [[nodiscard]] bool is_parsing_active() const;
    void set_parsing_mode(ParsingMode mode);
    [[nodiscard]] ParsingMode get_parsing_mode() const;

private:
    const std::shared_ptr<Context> parent;

    Options options;
    SymbolTable symbol_table;
    ParsingMode parsing_mode;
};

#endif //INC_8008_ASSEMBLER_CONTEXT_H
