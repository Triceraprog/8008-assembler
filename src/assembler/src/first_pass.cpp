#include "first_pass.h"

#include "context.h"
#include "errors.h"
#include "files/file_reader.h"
#include "instruction.h"
#include "parsed_line.h"
#include "parsed_line_storage.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <tuple>

namespace
{
    void throws_if_already_defined(const Context& context, const std::string& label)
    {
        if (auto symbol_value = context.get_symbol_value(label); std::get<0>(symbol_value))
        {
            throw AlreadyDefinedSymbol(label, std::get<1>(symbol_value));
        }
    }

    void define_symbol_or_fail(Context& context, const std::string& label, const int line_address,
                               const Instruction& instruction)
    {
        const auto& options = context.get_options();

        throws_if_already_defined(context, label);

        auto optional_value = instruction.get_value_for_label(context, line_address);

        if (optional_value.has_value())
        {
            auto& value = optional_value.value();
            context.define_symbol(label, value);

            if (options.debug)
            {
                std::cout << "at address=" << line_address;
                std::cout << std::hex << std::uppercase << " (= 0x" << line_address;
                std::cout << ") defining " << label << " = " << std::dec << value;
                std::cout << " (= 0x" << std::hex << std::uppercase << value << ")\n";
            }
        }
    }

    void handle_potential_label(Context& context, const ParsedLine& parsed_line)
    {
        if (!parsed_line.tokens.label.empty())
        {
            define_symbol_or_fail(context, parsed_line.tokens.label, parsed_line.line_address,
                                  parsed_line.instruction);
        }
    }

    bool is_exiting_macro(const ContextStack& context_stack, const ParsedLineStorage& parsed_lines)
    {
        const auto& tokens = parsed_lines.latest_line().tokens;

        return (context_stack.get_current_context()->get_parsing_mode() ==
                Context::MACRO_RECORDING) &&
               ci_equals(tokens.opcode, ".endmacro");
    }

    int first_pass_execution(ContextStack& context_stack, const ParsedLine& latest_parsed_line,
                             int current_address)
    {
        handle_potential_label(*context_stack.get_current_context(), latest_parsed_line);
        const auto& instruction = latest_parsed_line.instruction;
        return instruction.first_pass(context_stack, current_address);
    }

}

void first_pass(ContextStack context_stack, FileReader& file_reader,
                ParsedLineStorage& parsed_line_storage)
{
    // In the first pass, we parse through lines to build a symbol table
    // What is parsed is kept into the "parsed_lines" container for the second pass.
    const auto& options = context_stack.get_current_context()->get_options();

    if (options.debug || options.verbose)
    {
        std::cout << "Pass number One:  Read and Define Symbols\n";
    }

    int current_address = 0;
    for (const std::string& input_line : file_reader)
    {
        if (options.verbose || options.debug)
        {
            std::cout << "     0x" << std::hex << std::uppercase << current_address << " ";
            std::cout << "\"" << input_line << "\"\n";
        }

        try
        {
            parsed_line_storage.append_line(context_stack.get_current_context(), file_reader,
                                            input_line, file_reader.get_line_number(),
                                            current_address);

            const bool exiting_macro = is_exiting_macro(context_stack, parsed_line_storage);
            const Context::ParsingMode parsing_mode =
                    context_stack.get_current_context()->get_parsing_mode();
            if ((parsing_mode != Context::MACRO_RECORDING) || exiting_macro)
            {
                const auto& latest_parsed_line = parsed_line_storage.latest_line();
                current_address =
                        first_pass_execution(context_stack, latest_parsed_line, current_address);
            }
            else
            {
                context_stack.get_current_context()->record_macro_line(input_line);
            }
        }
        catch (const std::exception& ex)
        {
            throw ParsingException(ex, file_reader.get_line_number(), file_reader.get_name_tag(),
                                   input_line);
        }
    }
}

AlreadyDefinedSymbol::AlreadyDefinedSymbol(const std::string& symbol, int value)
{
    reason = "label '" + symbol + "' was already defined as " + std::to_string(value);
}
