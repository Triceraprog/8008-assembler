#include "context.h"

#include "macro_content.h"

#include <utility>

Context::Context(Options options) : options{std::move(options)} {}

Context::Context(const std::shared_ptr<Context>& other_context)
    : options{other_context->options}, parent{other_context}
{}

Context::~Context() = default;

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
bool Context::is_parsing_active() const
{
    return parsing_mode != CONDITIONAL_FALSE && parsing_mode != MACRO_RECORDING;
}
void Context::set_parsing_mode(Context::ParsingMode mode) { parsing_mode = mode; }
Context::ParsingMode Context::get_parsing_mode() const { return parsing_mode; }

void Context::declare_macro(std::unique_ptr<MacroContent> macro_content)
{
    std::string macro_name{macro_content->get_name()};
    std::transform(macro_name.begin(), macro_name.end(), macro_name.begin(), toupper);

    if (macros.contains(macro_name))
    {
        throw AlreadyDefinedMacro(macro_name);
    }
    macros[macro_name] = std::move(macro_content);
}

bool Context::has_macro(const std::string_view& macro_name) const
{
    std::string upper_macro_name{macro_name};
    std::transform(upper_macro_name.begin(), upper_macro_name.end(), upper_macro_name.begin(),
                   toupper);
    return macros.contains(std::string{upper_macro_name});
}

void* Context::create_call_context(std::string_view macro_name,
                                   const std::vector<std::string>& arguments) const
{
    return nullptr;
}

void Context::activate_macro(void* call_context) {}

AlreadyDefinedMacro::AlreadyDefinedMacro(const std::string& macro_name)
{
    reason = "macro '" + macro_name + "' was already defined";
}
