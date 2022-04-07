#include "context.h"

//Context::Context(const Options& options) : options(options) {}

void Context::define_symbol(std::string_view symbol_name, int value)
{
    symbol_table.define_symbol(symbol_name, value);
}

std::tuple<bool, int> Context::get_symbol_value(std::string_view symbol_name) const
{
    return symbol_table.get_symbol_value(symbol_name);
}
