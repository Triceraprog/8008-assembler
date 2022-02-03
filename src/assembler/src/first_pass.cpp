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

#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <tuple>

namespace
{
    void write_listing_header(const Options& options, FILE* lfp)
    {
        time_t result = time(nullptr);
        std::string compile_time{asctime(localtime(&result))};

        fprintf(lfp, "AS8 assembler for intel 8008, t.e.jones Version 1.0\n");
        fprintf(lfp, "Options: listfile=%d debug=%d ", options.generate_list_file, options.debug);
        fprintf(lfp, "binaryout=%d singlelist=%d\n", options.generate_binary_file,
                options.single_byte_list);
        fprintf(lfp, "octalnums=%d markascii=%d\n", options.input_num_as_octal,
                options.mark_8_ascii);
        fprintf(lfp, "Infile=%s\n", options.input_filename.c_str());
        fprintf(lfp, "Assembly Performed: %s\n\n", compile_time.c_str());
        if (options.single_byte_list)
        {
            fprintf(lfp, "Line Addr.  DAT Source Line\n");
            fprintf(lfp, "---- ------ --- ----------------------------------\n");
        }
        else
        {
            fprintf(lfp, "Line Addr.  CodeBytes   Source Line\n");
            fprintf(lfp, "---- ------ ----------- ----------------------------------\n");
        }
    }
}

void first_pass(const Options& options, SymbolTable& symbol_table, Files& files)
{
    /* In the first pass, we just parse through lines to build a symbol table */

    if (options.debug || options.verbose)
    {
        std::cout << "Pass number One:  Read and Define Symbols\n";
    }

    write_listing_header(options, files.lfp);

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
                    std::cerr << " in line " << current_line_count << " " << input_line;
                    std::cerr << " label " << tokens.label;
                    std::cerr << " already defined as " << std::get<1>(symbol_value) << "\n";
                    exit(-1);
                }

                /* Or define it. */
                int val;
                if (ci_equals(tokens.opcode, "equ") || ci_equals(tokens.opcode, "org"))
                {
                    val = evaluate_argument(options, symbol_table, current_line_count, tokens.arg1);
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
                if ((current_address = evaluate_argument(options, symbol_table, current_line_count,
                                                         tokens.arg1)) == -1)
                {
                    std::cerr << " in line " << current_line_count << " " << input_line;
                    std::cerr << " can't evaluate argument " << tokens.arg1 << "\n";
                    exit(-1);
                }
            }
            else if (ci_equals(tokens.opcode, "data"))
            {
                int data_list[80];
                int n = decode_data(options, symbol_table, current_line_count, input_line.c_str(),
                                    data_list);
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
        catch (const CannotFindSymbol& ex)
        {
            throw ParsingException(ex, current_line_count, input_line);
        }
    }
}
