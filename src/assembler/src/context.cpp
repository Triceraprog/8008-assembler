#include "context.h"

#include <utility>

Context::Context(Options options) : options{std::move(options)} {}

Context::Context(const std::shared_ptr<Context>& other_context)
    : options{other_context->options}, parent{other_context}
{}

void Context::define_symbol(std::string_view symbol_name, int value)
{
    symbol_table.define_symbol(symbol_name, value);
}

std::tuple<bool, int> Context::get_symbol_value(std::string_view symbol_name) const
{
    const auto [success, value] = symbol_table.get_symbol_value(symbol_name);

    if (success)
    {
        return {success, value};
    }
    else if (parent)
    {
        return parent->get_symbol_value(symbol_name);
    }

    return {false, 0};
}

void Context::list_symbols(std::ostream& output) { symbol_table.list_symbols(output); }

Options& Context::get_options() { return options; }
const Options& Context::get_options() const { return options; }