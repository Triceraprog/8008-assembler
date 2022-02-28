#include "first_pass.h"

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
    void define_symbol_or_fail(const Options& options, SymbolTable& symbol_table,
                               const std::string& label, const int line_address,
                               const Instruction& instruction)
    {
        const auto current_address = line_address;
        /* Check if the label was already defined. */
        if (auto symbol_value = symbol_table.get_symbol_value(label); std::get<0>(symbol_value))
        {
            throw AlreadyDefinedSymbol(label, std::get<1>(symbol_value));
        }

        int val = instruction.get_evaluation(options, symbol_table, current_address);

        if (options.debug)
        {
            std::cout << "at address=" << current_address;
            std::cout << std::hex << std::uppercase << "=" << current_address;
            std::cout << " defining " << label << " = " << std::dec << val;
            std::cout << " =0x" << std::hex << std::uppercase << val << "\n";
        }

        symbol_table.define_symbol(label, val);
    }

    void handle_potential_label(const Options& options, SymbolTable& symbol_table,
                                const ParsedLine& parsed_line, const Instruction& instruction)
    {
        const auto& tokens = parsed_line.tokens;
        if (!tokens.label.empty())
        {
            define_symbol_or_fail(options, symbol_table, tokens.label, parsed_line.line_address,
                                  instruction);
        }
    }
}

void first_pass(const Options& options, SymbolTable& symbol_table, Files& files,
                std::vector<ParsedLine>& parsed_lines, Listing& listing)
{
    /* In the first pass, we just parse through lines to build a symbol table */
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

        {
            // "tokens" is in a scope to avoid leaking. It's only available through the
            // parsed_lines container. This is because instruction is taking a reference
            // on the arguments. That might need to be changed.
            LineTokenizer tokens = parse_line(options, input_line, current_line_count);
            parsed_lines.push_back({current_line_count, current_address, tokens, input_line});
        }

        Instruction instruction{parsed_lines.back().tokens.opcode,
                                parsed_lines.back().tokens.arguments};

        try
        {
            handle_potential_label(options, symbol_table, parsed_lines.back(), instruction);
            current_address = instruction.first_pass(options, symbol_table, current_address);
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

