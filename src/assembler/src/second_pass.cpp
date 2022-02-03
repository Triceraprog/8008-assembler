#include "second_pass.h"
#include "byte_writer.h"
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
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace
{
    void stream_rewind(std::fstream& fstream)
    {
        fstream.clear();
        fstream.seekg(std::ios::beg);
    }
}

void second_pass(const Options& options, const SymbolTable& symbol_table, Files& files)
{
    /* Symbols are defined. Second pass. */
    int arg1;

    if (options.verbose || options.debug)
    {
        std::cout << "Pass number Two:  Re-read and assemble codes\n";
    }

    char single_space_pad[9]; /* this is some extra padding if we use single space list file */
    if (options.single_byte_list)
    {
        single_space_pad[0] = 0;
    }

    else
    {
        strcpy(single_space_pad, "        ");
    }

    int current_line_count = 0;
    int current_address = 0;
    int line_address;

    ByteWriter writer(files.output_stream,
                      options.generate_binary_file ? ByteWriter::BINARY : ByteWriter::HEX);

    stream_rewind(files.input_stream);
    for (std::string input_line; std::getline(files.input_stream, input_line);)
    {
        try
        {

            line_address = current_address;
            current_line_count++;

            if (options.verbose || options.debug)
            {
                printf("     0x%X \"%s\"\n", current_address, input_line.c_str());
            }

            LineTokenizer tokens = parse_line(options, input_line, current_line_count);
            auto args = tokens.arg_count;

            if (tokens.opcode.empty())
            {
                /* Must just be a comment line (or label only) */
                if (options.generate_list_file)
                {
                    fprintf(files.lfp, "%4d            %s%s\n", current_line_count,
                            single_space_pad, input_line.c_str());
                }
                continue;
            }

            /* Check if this opcode is one of the pseudo ops. */
            if (ci_equals(tokens.opcode, "equ"))
            {
                if (options.generate_list_file)
                    fprintf(files.lfp, "%4d            %s%s\n", current_line_count,
                            single_space_pad, input_line.c_str());
                continue;
            }
            if (ci_equals(tokens.opcode, "cpu"))
            {
                if (options.generate_list_file)
                    fprintf(files.lfp, "%4d            %s%s\n", current_line_count,
                            single_space_pad, input_line.c_str());
                continue;
            }

            if (ci_equals(tokens.opcode, "org"))
            {
                if ((current_address = evaluate_argument(options, symbol_table, tokens.arg1)) == -1)
                {
                    fprintf(stderr, " in line %d %s can't evaluate argument %s\n",
                            current_line_count, input_line.c_str(), tokens.arg1.c_str());
                    exit(-1);
                }
                if (options.generate_list_file)
                    fprintf(files.lfp, "%4d            %s%s\n", current_line_count,
                            single_space_pad, input_line.c_str());
                continue;
            }
            if (ci_equals(tokens.opcode, "end"))
            {
                if (options.generate_list_file)
                    fprintf(files.lfp, "%4d            %s%s\n", current_line_count,
                            single_space_pad, input_line.c_str());
                /* could break here, but rather than break, */
                /* we will go ahead and check for more with a continue */
                continue;
            }
            else if (ci_equals(tokens.opcode, "data"))
            {
                int data_list[80];
                int n = decode_data(options, symbol_table, current_line_count, input_line.c_str(),
                                    data_list);
                /* if n is negative, that number of bytes are just reserved */
                if (n < 0)
                {
                    if (options.generate_list_file)
                        fprintf(files.lfp, "%4d %02o-%03o     %s%s\n", current_line_count,
                                ((line_address >> 8) & 0xFF), (line_address & 0xFF),
                                single_space_pad, input_line.c_str());
                    current_address += 0 - n;
                    continue;
                }
                for (int i = 0; i < n; i++)
                {
                    writer.write_byte(data_list[i], current_address++);
                }
                if (options.generate_list_file)
                {
                    if (options.single_byte_list)
                    {
                        fprintf(files.lfp, "%4d %02o-%03o %03o %s\n", current_line_count,
                                ((line_address >> 8) & 0xFF), (line_address & 0xFF), data_list[0],
                                input_line.c_str());
                        for (int i = 1; i < n; i++)
                        {
                            fprintf(files.lfp, "%4d %02o-%03o %03o\n", current_line_count,
                                    (((line_address + i) >> 8) & 0xFF), ((line_address + i) & 0xFF),
                                    data_list[i]);
                        }
                    }
                    else
                    {
                        fprintf(files.lfp, "%4d %02o-%03o ", current_line_count,
                                ((line_address >> 8) & 0xFF), (line_address & 0xFF));
                        if (n == 1)
                            fprintf(files.lfp, "%03o          %s\n", data_list[0],
                                    input_line.c_str());
                        else if (n == 2)
                            fprintf(files.lfp, "%03o %03o      %s\n", data_list[0], data_list[1],
                                    input_line.c_str());
                        else if (n > 2)
                        {
                            fprintf(files.lfp, "%03o %03o %03o  %s\n", data_list[0], data_list[1],
                                    data_list[2], input_line.c_str());
                            int* ptr = data_list + 3;
                            n -= 3;
                            line_address += 3;
                            while (n > 0)
                            {
                                /*	    fprintf(files.lfp,"            "); */
                                fprintf(files.lfp, "     %02o-%03o ", ((line_address >> 8) & 0xFF),
                                        (line_address & 0xFF));
                                if (n > 2)
                                {
                                    fprintf(files.lfp, "%03o %03o %03o\n", ptr[0], ptr[1], ptr[2]);
                                    ptr += 3;
                                    n -= 3;
                                    line_address += 3;
                                }
                                else
                                {
                                    for (int i = 0; i < n; i++)
                                    {
                                        fprintf(files.lfp, "%03o ", ptr[0]);
                                        ptr++;
                                    }
                                    n = 0;
                                    fprintf(files.lfp, "\n");
                                }
                            }
                        }
                    }
                }
                continue;
            }
            /* Now we should have an opcode. */

            auto [found, opcode] = find_opcode(tokens.opcode);
            if (!found)
            {
                fprintf(stderr, " in line %d %s undefined opcode %s\n", current_line_count,
                        input_line.c_str(), tokens.opcode.c_str());
                exit(-1);
            }
            /* found the opcode */
            /* check that we have right number of arguments */
            if (((opcode.rule == 0) && (args != 0)) || ((opcode.rule == 1) && (args != 1)) ||
                ((opcode.rule == 2) && (args != 1)) || ((opcode.rule == 3) && (args != 1)) ||
                ((opcode.rule == 4) && (args != 1)))
            {
                fprintf(stderr, " in line %d %s we see an unexpected %d arguments\n",
                        current_line_count, input_line.c_str(), args);
                exit(-1);
            }
            if (args == 1)
            {
                if ((arg1 = evaluate_argument(options, symbol_table, tokens.arg1)) == -1)
                {
                    fprintf(stderr, " in line %d %s can't evaluate argument %s\n",
                            current_line_count, input_line.c_str(), tokens.arg1.c_str());
                    exit(-1);
                }
            }
            /* Now, each opcode, is categorized into different
         * "rules" which states how arguments are combined
         * with opcode to get machine codes. */

            if (opcode.rule == 0)
            {
                /* single byte, no arguments */
                writer.write_byte(opcode.code, current_address++);
                if (options.generate_list_file)
                    fprintf(files.lfp, "%4d %02o-%03o %03o %s%s\n", current_line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF), opcode.code,
                            single_space_pad, input_line.c_str());
            }
            else if (opcode.rule == 1)
            {
                /* single byte, must follow */
                if ((arg1 > 255) || (arg1 < 0))
                {
                    fprintf(stderr, " in line %d %s expected argument 0-255\n", current_line_count,
                            input_line.c_str());
                    fprintf(stderr, "    instead got %s=%d\n", tokens.arg1.c_str(), arg1);
                    exit(-1);
                }
                int code = opcode.code;
                writer.write_byte(code, current_address++);
                writer.write_byte(arg1, current_address++);
                if (options.generate_list_file)
                {
                    if (options.single_byte_list)
                    {
                        fprintf(files.lfp, "%4d %02o-%03o %03o %s\n", current_line_count,
                                ((line_address >> 8) & 0xFF), (line_address & 0xFF), code,
                                input_line.c_str());
                        line_address++;
                        fprintf(files.lfp, "     %02o-%03o %03o\n", (((line_address) >> 8) & 0xFF),
                                ((line_address) &0xFF), arg1);
                    }
                    else
                    {
                        fprintf(files.lfp, "%4d %02o-%03o %03o %03o     %s\n", current_line_count,
                                ((line_address >> 8) & 0xFF), (line_address & 0xFF), code, arg1,
                                input_line.c_str());
                    }
                }
            }
            else if (opcode.rule == 2)
            {
                /* two byte address to follow */
                if ((arg1 > 1024 * 16) || (arg1 < 0))
                {
                    fprintf(stderr, " in input_line.c_str() %d %s expected argument 0-%d\n",
                            current_line_count, input_line.c_str(), 1024 * 16);
                    fprintf(stderr, "    instead got %s=%d\n", tokens.arg1.c_str(), arg1);
                    exit(-1);
                }
                int code = opcode.code;
                int lowbyte = (0xFF & arg1);
                int highbyte = (0xFF & (arg1 >> 8));
                writer.write_byte(code, current_address++);
                writer.write_byte(lowbyte, current_address++);
                writer.write_byte(highbyte, current_address++);
                if (options.generate_list_file)
                {
                    if (options.single_byte_list)
                    {
                        fprintf(files.lfp, "%4d %02o-%03o %03o %s\n", current_line_count,
                                ((line_address >> 8) & 0xFF), (line_address & 0xFF), code,
                                input_line.c_str());
                        line_address++;
                        fprintf(files.lfp, "     %02o-%03o %03o\n", ((line_address >> 8) & 0xFF),
                                (line_address & 0xFF), lowbyte);
                        line_address++;
                        fprintf(files.lfp, "     %02o-%03o %03o\n", ((line_address >> 8) & 0xFF),
                                (line_address & 0xFF), highbyte);
                    }
                    else
                    {
                        fprintf(files.lfp, "%4d %02o-%03o %03o %03o %03o %s\n", current_line_count,
                                ((line_address >> 8) & 0xFF), (line_address & 0xFF), code, lowbyte,
                                highbyte, input_line.c_str());
                    }
                }
            }
            else if (opcode.rule == 3)
            {
                /* have an input or output instruction */
                int maxport;

                if (opcode.mnemonic[0] == 'i')
                    maxport = 7;
                else
                    maxport = 23;
                if ((arg1 > maxport) || (arg1 < 0))
                {
                    fprintf(stderr, " in input_line.c_str() %d %s expected port 0-%d\n",
                            current_line_count, input_line.c_str(), maxport);
                    fprintf(stderr, "    instead got %s=%d\n", tokens.arg1.c_str(), arg1);
                    exit(-1);
                }
                int code = opcode.code + (arg1 << 1);
                writer.write_byte(code, current_address++);
                if (options.generate_list_file)
                    fprintf(files.lfp, "%4d %02o-%03o %03o %s%s\n", current_line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF), code,
                            single_space_pad, input_line.c_str());
            }
            else if (opcode.rule == 4)
            {
                if ((arg1 > 7) || (arg1 < 0))
                {
                    fprintf(stderr, " in line %d %s expected argument 0-7\n", current_line_count,
                            input_line.c_str());
                    fprintf(stderr, "    instead got %s=%d\n", tokens.arg1.c_str(), arg1);
                    exit(-1);
                }
                auto code = (opcode.code | (arg1 << 3));
                writer.write_byte(code, current_address++);
                if (options.generate_list_file)
                    fprintf(files.lfp, "%4d %02o-%03o %03o %s%s\n", current_line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF), code,
                            single_space_pad, input_line.c_str());
            }
            else
            {
                fprintf(stderr, " in input_line.c_str() %d %s can't comprehend rule %d\n",
                        current_line_count, input_line.c_str(), opcode.rule);
                exit(-1);
            }
        }
        catch (const CannotFindSymbol& ex)
        {
            throw ParsingException(ex, current_line_count, input_line);
        }
    }

    writer.write_end();
}
