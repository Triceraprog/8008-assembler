#include "context.h"

#include "files/file_reader.h"
#include "macro_content.h"

#include <cassert>
#include <sstream>
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

    assert(!macros.contains(macro_name)); // Because starting the macro already verifies it.
    macros[macro_name] = std::move(macro_content);
}

bool Context::has_macro(const std::string_view& macro_name) const
{
    std::string upper_macro_name{macro_name};
    std::transform(upper_macro_name.begin(), upper_macro_name.end(), upper_macro_name.begin(),
                   toupper);
    return macros.contains(std::string{upper_macro_name});
}

MacroContent* Context::get_macro_content(std::string_view macro_name) const
{
    std::string upper_macro_name{macro_name};
    std::transform(upper_macro_name.begin(), upper_macro_name.end(), upper_macro_name.begin(),
                   toupper);

    auto it = macros.find(upper_macro_name);
    return it->second.get();
}

void Context::call_macro(MacroContent* macro_content, const std::vector<std::string>& arguments,
                         FileReader& file_reader, const std::function<void()>& callback)
{
    // Create the parameters/arguments association
    const auto& parameters = macro_content->get_parameters();
    auto arg_it = std::begin(arguments);

    for (const auto& param : parameters)
    {
        macro_param_arg_association[param] = *arg_it;
        ++arg_it;
    }

    // Insert the content of the macro in the input lines
    auto stream = macro_content->get_line_stream();
    file_reader.insert_now(std::move(stream), macro_content->get_name(), callback);
}

void Context::start_macro(const std::string& macro_name, const std::vector<std::string>& arguments)
{
    assert(currently_recording_macro.get() == nullptr);
    assert(get_parsing_mode() != Context::MACRO_RECORDING);

    if (parent && parent->has_macro(macro_name))
    {
        throw AlreadyDefinedMacro(macro_name);
    }
    set_parsing_mode(Context::MACRO_RECORDING);
    currently_recording_macro = std::make_unique<MacroContent>(macro_name, arguments);
}

void Context::stop_macro()
{
    if (parent)
    {
        parent->declare_macro(std::move(currently_recording_macro));
    }
}

void Context::record_macro_line(const std::string& line)
{
    assert(currently_recording_macro.get() != nullptr);
    assert(get_parsing_mode() == Context::MACRO_RECORDING);

    currently_recording_macro->append_line(line);
}

void Context::replace_macro_tokens(std::vector<std::string>& tokens)
{
    for (auto& token : tokens)
    {
        std::string lowercase{token};
        std::transform(std::begin(lowercase), std::end(lowercase), std::begin(lowercase),
                       ::tolower);
        auto it = macro_param_arg_association.find(lowercase);

        if (it != std::end(macro_param_arg_association))
        {
            token = it->second;
        }
    }
}

AlreadyDefinedMacro::AlreadyDefinedMacro(const std::string& macro_name)
{
    reason = "macro '" + macro_name + "' was already defined";
}
