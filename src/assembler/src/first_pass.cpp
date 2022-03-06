#include "first_pass.h"

#include "context.h"
#include "errors.h"
#include "files.h"
#include "instruction.h"
#include "line_tokenizer.h"
#include "options.h"
#include "parsed_line.h"
#include "symbol_table.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <tuple>

namespace
{
    void throws_if_already_defined(const SymbolTable& symbol_table, const std::string& label)
    {
        if (auto symbol_value = symbol_table.get_symbol_value(label); std::get<0>(symbol_value))
        {
            throw AlreadyDefinedSymbol(label, std::get<1>(symbol_value));
        }
    }

    void define_symbol_or_fail(const Context& context, const Options& options,
                               SymbolTable& symbol_table, const std::string& label,
                               const int line_address, const Instruction& instruction)
    {
        throws_if_already_defined(symbol_table, label);

        int val = instruction.get_evaluation(context, options, symbol_table, line_address);
        symbol_table.define_symbol(label, val);

        if (options.debug)
        {
            std::cout << "at address=" << line_address;
            std::cout << std::hex << std::uppercase << "=" << line_address;
            std::cout << " defining " << label << " = " << std::dec << val;
            std::cout << " =0x" << std::hex << std::uppercase << val << "\n";
        }
    }

    void handle_potential_label(const Context& context, const Options& options,
                                SymbolTable& symbol_table, const ParsedLine& parsed_line)
    {
        if (!parsed_line.tokens.label.empty())
        {
            define_symbol_or_fail(context, options, symbol_table, parsed_line.tokens.label,
                                  parsed_line.line_address, parsed_line.instruction);
        }
    }
}

void first_pass(const Context& context, Files& files, SymbolTable& symbol_table,
                std::vector<ParsedLine>& parsed_lines)
{
    // In the first pass, we parse through lines to build a symbol table
    // What is parsed is kept into the "parsed_lines" container for the second pass.
    const auto& options = context.options;
    if (options.debug || options.verbose)
    {
        std::cout << "Pass number One:  Read and Define Symbols\n";
    }

    int current_line_count = 0;
    int current_address = 0;
    for (std::string input_line; std::getline(files.input_stream, input_line);)
    {
        current_line_count++;
        if (options.verbose || options.debug)
        {
            std::cout << "     0x" << std::hex << std::uppercase << current_address << " ";
            std::cout << "\"" << input_line << "\"\n";
        }

        try
        {
            LineTokenizer tokens = parse_line(options, input_line, current_line_count);
            {
                Instruction instruction{context, tokens.opcode, tokens.arguments};
                parsed_lines.push_back({current_line_count, current_address, tokens,
                                        std::move(instruction), input_line});
            }

            handle_potential_label(context, options, symbol_table, parsed_lines.back());
            auto& instruction = parsed_lines.back().instruction;
            current_address = instruction.first_pass(context, current_address);
        }
        catch (const std::exception& ex)
        {
            throw ParsingException(ex, current_line_count, input_line);
        }
    }
}

AlreadyDefinedSymbol::AlreadyDefinedSymbol(const std::string& symbol, int value)
{
    reason = "label '" + symbol + "' was already defined as " + std::to_string(value);
}
