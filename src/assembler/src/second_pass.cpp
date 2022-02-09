#include "second_pass.h"
#include "byte_writer.h"
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

    bool correct_argument_count(const Opcode& opcode, uint32_t arg_count)
    {
        return ((opcode.rule == 0) && (arg_count != 0)) ||
               ((opcode.rule == 1) && (arg_count != 1)) ||
               ((opcode.rule == 2) && (arg_count != 1)) ||
               ((opcode.rule == 3) && (arg_count != 1)) || ((opcode.rule == 4) && (arg_count != 1));
    }
}

void second_pass(const Options& options, const SymbolTable& symbol_table, Files& files,
                 Listing& listing)
{
    /* Symbols are defined. Second pass. */
    int evaluated_arg1;

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
            auto arg_count = tokens.arg_count;

            switch (opcode_to_enum(tokens.opcode))
            {
                case PseudoOpcodeEnum::ORG:
                    current_address = evaluate_argument(options, symbol_table, tokens.arg1);
                    // no break
                case PseudoOpcodeEnum::EMPTY:
                case PseudoOpcodeEnum::EQU:
                case PseudoOpcodeEnum::CPU:
                case PseudoOpcodeEnum::END:
                    /* For END, could break here, but rather than break, */
                    /* we will go ahead and check for more. */
                    if (options.generate_list_file)
                    {
                        listing.simple_line(current_line_count, input_line);
                    }
                    break;
                case PseudoOpcodeEnum::DATA: {
                    std::vector<int> data_list;
                    int data_length =
                            decode_data(options, symbol_table, input_line.c_str(), data_list);
                    if (data_length < 0)
                    {
                        /* if n is negative, that number of bytes are just reserved */
                        if (options.generate_list_file)
                        {
                            listing.reserved_data(current_line_count, line_address, input_line);
                        }
                        current_address += 0 - data_length;
                    }
                    else
                    {
                        for (const auto& data : data_list)
                        {
                            writer.write_byte(data, current_address);
                            current_address += 1;
                        }
                        if (options.generate_list_file)
                        {
                            listing.data(current_line_count, line_address, input_line, data_list);
                        }
                    }
                }
                break;
                case PseudoOpcodeEnum::OTHER: {
                    auto [found, opcode] = find_opcode(tokens.opcode);
                    if (!found)
                    {
                        throw UndefinedOpcode(tokens.opcode);
                    }
                    /* found the opcode */
                    /* check that we have right the number of arguments */
                    if (correct_argument_count(opcode, arg_count))
                    {
                        fprintf(stderr, " in line %d %s we see an unexpected %d arguments\n",
                                current_line_count, input_line.c_str(), arg_count);
                        exit(-1);
                    }
                    if (arg_count == 1)
                    {
                        evaluated_arg1 = evaluate_argument(options, symbol_table, tokens.arg1);
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
                                    ((line_address >> 8) & 0xFF), (line_address & 0xFF),
                                    opcode.code, single_space_pad, input_line.c_str());
                    }
                    else if (opcode.rule == 1)
                    {
                        /* single byte, must follow */
                        if ((evaluated_arg1 > 255) || (evaluated_arg1 < 0))
                        {
                            throw ExpectedArgumentWithinLimits(255, tokens.arg1, evaluated_arg1);
                        }
                        int code = opcode.code;
                        writer.write_byte(code, current_address++);
                        writer.write_byte(evaluated_arg1, current_address++);
                        if (options.generate_list_file)
                        {
                            if (options.single_byte_list)
                            {
                                fprintf(files.lfp, "%4d %02o-%03o %03o %s\n", current_line_count,
                                        ((line_address >> 8) & 0xFF), (line_address & 0xFF), code,
                                        input_line.c_str());
                                line_address++;
                                fprintf(files.lfp, "     %02o-%03o %03o\n",
                                        (((line_address) >> 8) & 0xFF), ((line_address) &0xFF),
                                        evaluated_arg1);
                            }
                            else
                            {
                                fprintf(files.lfp, "%4d %02o-%03o %03o %03o     %s\n",
                                        current_line_count, ((line_address >> 8) & 0xFF),
                                        (line_address & 0xFF), code, evaluated_arg1,
                                        input_line.c_str());
                            }
                        }
                    }
                    else if (opcode.rule == 2)
                    {
                        /* two byte address to follow */
                        const int MAX_ADDRESS = 1024 * 16;
                        if ((evaluated_arg1 > MAX_ADDRESS) || (evaluated_arg1 < 0))
                        {
                            throw ExpectedArgumentWithinLimits(MAX_ADDRESS, tokens.arg1,
                                                               evaluated_arg1);
                        }
                        int code = opcode.code;
                        int low_byte = (0xFF & evaluated_arg1);
                        int high_byte = (0xFF & (evaluated_arg1 >> 8));
                        writer.write_byte(code, current_address++);
                        writer.write_byte(low_byte, current_address++);
                        writer.write_byte(high_byte, current_address++);
                        if (options.generate_list_file)
                        {
                            if (options.single_byte_list)
                            {
                                fprintf(files.lfp, "%4d %02o-%03o %03o %s\n", current_line_count,
                                        ((line_address >> 8) & 0xFF), (line_address & 0xFF), code,
                                        input_line.c_str());
                                line_address++;
                                fprintf(files.lfp, "     %02o-%03o %03o\n",
                                        ((line_address >> 8) & 0xFF), (line_address & 0xFF),
                                        low_byte);
                                line_address++;
                                fprintf(files.lfp, "     %02o-%03o %03o\n",
                                        ((line_address >> 8) & 0xFF), (line_address & 0xFF),
                                        high_byte);
                            }
                            else
                            {
                                fprintf(files.lfp, "%4d %02o-%03o %03o %03o %03o %s\n",
                                        current_line_count, ((line_address >> 8) & 0xFF),
                                        (line_address & 0xFF), code, low_byte, high_byte,
                                        input_line.c_str());
                            }
                        }
                    }
                    else if (opcode.rule == 3)
                    {
                        /* have an input or output instruction */
                        int max_port = (opcode.mnemonic[0] == 'i') ? 7 : 23;

                        if ((evaluated_arg1 > max_port) || (evaluated_arg1 < 0))
                        {
                            throw ExpectedArgumentWithinLimits(max_port, tokens.arg1,
                                                               evaluated_arg1);
                        }

                        int code = opcode.code + (evaluated_arg1 << 1);
                        writer.write_byte(code, current_address++);
                        if (options.generate_list_file)
                            fprintf(files.lfp, "%4d %02o-%03o %03o %s%s\n", current_line_count,
                                    ((line_address >> 8) & 0xFF), (line_address & 0xFF), code,
                                    single_space_pad, input_line.c_str());
                    }
                    else if (opcode.rule == 4)
                    {
                        if ((evaluated_arg1 > 7) || (evaluated_arg1 < 0))
                        {
                            throw ExpectedArgumentWithinLimits(7, tokens.arg1, evaluated_arg1);
                        }

                        auto code = (opcode.code | (evaluated_arg1 << 3));
                        writer.write_byte(code, current_address++);
                        if (options.generate_list_file)
                            fprintf(files.lfp, "%4d %02o-%03o %03o %s%s\n", current_line_count,
                                    ((line_address >> 8) & 0xFF), (line_address & 0xFF), code,
                                    single_space_pad, input_line.c_str());
                    }
                    else
                    {
                        fprintf(stderr, " in line %d %s can't comprehend rule %d\n",
                                current_line_count, input_line.c_str(), opcode.rule);
                        exit(-1);
                    }
                }
                break;
            }
        }
        catch (const CannotFindSymbol& ex)
        {
            throw ParsingException(ex, current_line_count, input_line);
        }
    }

    writer.write_end();
}

ExpectedArgumentWithinLimits::ExpectedArgumentWithinLimits(int limit, std::string& content,
                                                           int evaluated)
{
    reason = "expected argument between 0 and " + std::to_string(limit) + " instead got " +
             content + "=" + std::to_string(evaluated);
}
