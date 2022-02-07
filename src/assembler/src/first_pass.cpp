#include "first_pass.h"

#include "data_extraction.h"
#include "errors.h"
#include "evaluator.h"
#include "files.h"
#include "line_tokenizer.h"
#include "opcodes.h"
#include "options.h"
#include "symbol_table.h"
#include "utils.h"
#include "listing.h"

#include <iostream>
#include <string>
#include <tuple>

void first_pass(const Options& options, SymbolTable& symbol_table, Files& files, Listing & listing)
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
        try
        {
            current_line_count++;
            if (options.verbose || options.debug)
            {
                std::cout << "     0x" << std::hex << std::uppercase << current_address << " ";
                std::cout << "\"" << input_line << "\"\n";
            }
            /* this function breaks line into separate parts */
            LineTokenizer tokens = parse_line(options, input_line, current_line_count);

            if (options.debug)
            {
                tokens.debug_print();
            }

            if (!tokens.label.empty())
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

            if (tokens.opcode.empty() || ci_equals(tokens.opcode, "equ") ||
                ci_equals(tokens.opcode, "end"))
            {
                continue;
            }

            if (ci_equals(tokens.opcode, "cpu"))
            {
                if ((!ci_equals(tokens.arg1, "8008")) && (!ci_equals(tokens.arg1, "i8008")))
                {
                    std::cerr << " in line " << current_line_count << " " << input_line;
                    std::cerr << " cpu only allowed is \"8008\" or \"i8008\"\n";
                    exit(-1);
                }
                continue;
            }

            if (ci_equals(tokens.opcode, "org"))
            {
                current_address = evaluate_argument(options, symbol_table, tokens.arg1);
            }
            else if (ci_equals(tokens.opcode, "data"))
            {
                int data_list[80];
                int n = decode_data(options, symbol_table, input_line.c_str(), data_list);
                if (options.debug)
                {
                    std::cout << "got " << n << " items in data list\n";
                }

                /* a negative number denotes that much space to save, but not specifying data */
                /* if so, just change sign to positive */
                if (n < 0)
                {
                    n = 0 - n;
                }
                current_address += n;
                continue;
            }
            else if (auto [found, opcode] = find_opcode(tokens.opcode); found)
            {
                /* found the opcode */
                switch (opcode.rule)
                {
                    case 0:
                    case 3:
                    case 4:
                        current_address += 1;
                        break;
                    case 1:
                        current_address += 2;
                        break;
                    case 2:
                        current_address += 3;
                        break;
                }
            }
            else
            {
                std::cerr << " in line " << current_line_count << " " << input_line;
                std::cerr << " undefined opcode " << tokens.opcode << "\n";
                exit(-1);
            }
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
