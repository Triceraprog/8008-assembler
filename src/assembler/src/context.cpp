#include "context.h"

#include "options.h"
#include "symbol_table.h"

Context::Context(const Options& options)
{
    option_stack.push(options);
    symbol_tables.emplace_front();
}

void Context::define_symbol(std::string_view symbol_name, int value)
{
    symbol_tables.front().define_symbol(symbol_name, value);
}

std::tuple<bool, int> Context::get_symbol_value(std::string_view symbol_name) const
{
    return symbol_tables.front().get_symbol_value(symbol_name);
}

void Context::list_symbols(std::ostream& output) { symbol_tables.front().list_symbols(output); }

Options& Context::get_options() { return option_stack.top(); }
const Options& Context::get_options() const { return option_stack.top(); }

void Context::push()
{
    option_stack.push(option_stack.top());
    symbol_tables.emplace_front(symbol_tables.front());
}

void Context::pop()
{
    option_stack.pop();
    symbol_tables.pop_front();
}
