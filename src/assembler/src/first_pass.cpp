#include "first_pass.h"

#include "data_extraction.h"
#include "errors.h"
#include "evaluator.h"
#include "files.h"
#include "line_tokenizer.h"
#include "listing.h"
#include "opcodes.h"
#include "options.h"
#include "symbol_table.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <tuple>

namespace
{

    void define_symbol_or_fail(const Options& options, SymbolTable& symbol_table,
                               LineTokenizer& tokens, int current_address)
    {
        /* Check if the label was already defined. */
        if (auto symbol_value = symbol_table.get_symbol_value(tokens.label);
            std::get<0>(symbol_value))
        {
            throw AlreadyDefinedSymbol(tokens.label, std::get<1>(symbol_value));
        }

        /* Or define it. */
        int val;
        if (ci_equals(tokens.opcode, "equ") || ci_equals(tokens.opcode, "org"))
        {
            val = evaluate_argument(options, symbol_table, tokens.arg1);
        }
        else
        {
            val = current_address;
        }

        if (options.debug)
        {
            std::cout << "at address=" << current_address;
            std::cout << std::hex << std::uppercase << "=" << current_address;
            std::cout << " defining " << tokens.label << " = " << std::dec << val;
            std::cout << " =0x" << std::hex << std::uppercase << val << "\n";
        }

        symbol_table.define_symbol(tokens.label, val);
    }

    int get_opcode_size(const Opcode& opcode)
    {
        int opcode_byte_size;
        switch (opcode.rule)
        {
            case 0:
            case 3:
            case 4:
                opcode_byte_size = 1;
                break;
            case 1:
                opcode_byte_size = 2;
                break;
            case 2:
                opcode_byte_size = 3;
                break;
        }
        return opcode_byte_size;
    }

    void verify_cpu(const std::string& cpu_arg)
    {
        if ((!ci_equals(cpu_arg, "8008")) && (!ci_equals(cpu_arg, "i8008")))
        {
            throw InvalidCPU();
        }
    }
}

void first_pass(const Options& options, SymbolTable& symbol_table, Files& files, Listing& listing)
{
    /* In the first pass, we just parse through lines to build a symbol table */
    if (options.debug || options.verbose)
    {
        std::cout << "Pass number One:  Read and Define Symbols\n";
    }

    listing.write_listing_header();

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

        /* this function breaks line into separate parts */
        LineTokenizer tokens = parse_line(options, input_line, current_line_count);

        if (!tokens.label.empty())
        {
            try
            {
                define_symbol_or_fail(options, symbol_table, tokens, current_address);
            }
            catch (const std::exception& ex)
            {
                throw ParsingException(ex, current_line_count, input_line);
            }
        }

        switch (opcode_to_enum(tokens.opcode))
        {
            case PseudoOpcodeEnum::EMPTY:
            case PseudoOpcodeEnum::EQU:
            case PseudoOpcodeEnum::END:
                break;
            case PseudoOpcodeEnum::CPU:
                verify_cpu(tokens.arg1);
                break;
            case PseudoOpcodeEnum::ORG:
                try
                {
                    current_address = evaluate_argument(options, symbol_table, tokens.arg1);
                }
                catch (const std::exception& ex)
                {
                    throw ParsingException(ex, current_line_count, input_line);
                }
                break;
            case PseudoOpcodeEnum::DATA: {
                int data_size;
                try
                {
                    std::vector<int> data_list;
                    data_size = decode_data(options, symbol_table, input_line.c_str(), data_list);
                }
                catch (const std::exception& ex)
                {
                    throw ParsingException(ex, current_line_count, input_line);
                }

                if (options.debug)
                {
                    std::cout << "got " << data_size << " items in data list\n";
                }

                /* a negative number denotes that much space to save, but not specifying data */
                current_address += std::abs(data_size);
            }
            break;
            case PseudoOpcodeEnum::OTHER: {
                if (auto [found, opcode] = find_opcode(tokens.opcode); found)
                {
                    current_address += get_opcode_size(opcode);
                }
                else
                {
                    throw UndefinedOpcode(tokens.opcode);
                }
                break;
            }
        }
    }
}

AlreadyDefinedSymbol::AlreadyDefinedSymbol(const std::string& symbol, int value)
{
    reason = "label '" + symbol + "' was already defined as " + std::to_string(value);
}

InvalidCPU::InvalidCPU() { reason = R"(cpu only allowed is "8008" or "i8008")"; }
