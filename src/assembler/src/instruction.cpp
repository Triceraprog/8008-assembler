#include "instruction.h"
#include "byte_writer.h"
#include "data_extraction.h"
#include "evaluator.h"
#include "listing.h"
#include "opcode_action.h"
#include "options.h"
#include "symbol_table.h"
#include "utils.h"

#include <iostream>
#include <utility>

namespace
{
    void verify_cpu(const std::string& cpu_arg)
    {
        if ((!ci_equals(cpu_arg, "8008")) && (!ci_equals(cpu_arg, "i8008")))
        {
            throw InvalidCPU();
        }
    }

    bool correct_argument_count(const Opcode& opcode, uint32_t arg_count)
    {
        return ((opcode.rule == 0) && (arg_count != 0)) ||
               ((opcode.rule == 1) && (arg_count != 1)) ||
               ((opcode.rule == 2) && (arg_count != 1)) ||
               ((opcode.rule == 3) && (arg_count != 1)) || ((opcode.rule == 4) && (arg_count != 1));
    }
}

Instruction::Instruction(const std::string& opcode, std::vector<std::string> arguments)
    : opcode{opcode}, arguments{std::move(arguments)}
{
    opcode_enum = opcode_to_enum(opcode);
}

int Instruction::get_evaluation(const Options& options, const SymbolTable& symbol_table,
                                int current_address) const
{
    if (opcode_enum == PseudoOpcodeEnum::ORG || opcode_enum == PseudoOpcodeEnum::EQU)
    {
        return evaluate_argument(options, symbol_table, arguments[0]);
    }
    return current_address;
}

int Instruction::first_pass(const Options& options, const SymbolTable& symbol_table,
                            const int current_address) const
{
    int updated_address;
    switch (opcode_enum)
    {
        case PseudoOpcodeEnum::CPU:
            verify_cpu(arguments[0]);
        case PseudoOpcodeEnum::EMPTY:
        case PseudoOpcodeEnum::EQU:
        case PseudoOpcodeEnum::END:
            updated_address = current_address;
            break;
        case PseudoOpcodeEnum::ORG:
            updated_address = evaluate_argument(options, symbol_table, arguments[0]);
            break;

        case PseudoOpcodeEnum::DATA: {
            std::vector<int> data_list;
            int data_size = decode_data(options, symbol_table, arguments, data_list);

            if (options.debug)
            {
                std::cout << "got " << data_size << " items in data list\n";
            }

            /* a negative number denotes that much space to save, but not specifying data */
            updated_address = current_address + std::abs(data_size);
        }
        break;
        case PseudoOpcodeEnum::OTHER: {
            if (auto [found, found_opcode] = find_opcode(opcode); found)
            {
                updated_address = current_address + get_opcode_size(found_opcode);
            }
            else
            {
                throw UndefinedOpcode(opcode);
            }
            break;
        }
    }
    return updated_address;
}

void Instruction::second_pass(const Options& options, const SymbolTable& symbol_table,
                              Listing& listing, ByteWriter& writer, const std::string& input_line,
                              int line_number, const int address)
{
    if (opcode_enum == PseudoOpcodeEnum::DATA)
    {
        std::vector<int> data_list;
        const int data_length = decode_data(options, symbol_table, arguments, data_list);
        if (data_length < 0)
        {
            /* if n is negative, that number of bytes are just reserved */
            if (options.generate_list_file)
            {
                listing.reserved_data(line_number, address, input_line, options.single_byte_list);
            }
        }
        else
        {
            for (int write_address = address; const auto& data : data_list)
            {
                writer.write_byte(data, write_address);
                write_address += 1;
            }
            if (options.generate_list_file)
            {
                listing.data(line_number, address, input_line, data_list);
            }
        }
    }
    else if (opcode_enum == PseudoOpcodeEnum::OTHER)
    {
        auto arg_count = arguments.size();
        const auto [found, found_opcode] = find_opcode(opcode);
        if (!found)
        {
            throw UndefinedOpcode(opcode);
        }
        /* found the opcode */
        /* check that we have right the number of arguments */
        if (correct_argument_count(found_opcode, arg_count))
        {
            throw UnexpectedArgumentCount(arg_count);
        }

        // Now, each opcode, is categorized into different
        // "rules" which states how arguments are combined
        // with opcode to get machine codes.

        auto opcode_action =
                create_opcode_action(options, symbol_table, found_opcode, address, arguments);

        int current_address = address;
        int line_address = address;
        switch (found_opcode.rule)
        {
            case NO_ARG:
                opcode_action->emit_byte_stream(writer);
                if (options.generate_list_file)
                {
                    opcode_action->emit_listing(listing, line_number, input_line);
                }
                break;
            case ONE_BYTE_ARG: {
                int evaluated_arg1 = evaluate_argument(options, symbol_table, arguments[0]);
                if ((evaluated_arg1 > 255) || (evaluated_arg1 < 0))
                {
                    throw ExpectedArgumentWithinLimits(255, arguments[0], evaluated_arg1);
                }
                const int code = found_opcode.code;
                writer.write_byte(code, current_address++);
                writer.write_byte(evaluated_arg1, current_address);
                if (options.generate_list_file)
                {
                    if (options.single_byte_list)
                    {
                        listing.one_byte_of_data_with_address(line_number, line_address, code,
                                                              input_line);
                        line_address++;
                        listing.one_byte_of_data_continued(line_address, evaluated_arg1);
                    }
                    else
                    {

                        listing.opcode_line_with_space_1_arg(line_number, line_address,
                                                             found_opcode.code, evaluated_arg1,
                                                             input_line);
                    }
                }
            }
            break;
            case ADDRESS_ARG: {
                const int MAX_ADDRESS = 1024 * 16;
                int evaluated_arg1 = evaluate_argument(options, symbol_table, arguments[0]);
                if ((evaluated_arg1 > MAX_ADDRESS) || (evaluated_arg1 < 0))
                {
                    throw ExpectedArgumentWithinLimits(MAX_ADDRESS, arguments[0], evaluated_arg1);
                }
                const int code = found_opcode.code;
                const int low_byte = (0xFF & evaluated_arg1);
                const int high_byte = (0xFF & (evaluated_arg1 >> 8));
                writer.write_byte(code, current_address++);
                writer.write_byte(low_byte, current_address++);
                writer.write_byte(high_byte, current_address);
                if (options.generate_list_file)
                {
                    if (options.single_byte_list)
                    {
                        listing.one_byte_of_data_with_address(line_number, line_address, code,
                                                              input_line);
                        line_address++;
                        listing.one_byte_of_data_continued(line_address, low_byte);
                        line_address++;
                        listing.one_byte_of_data_continued(line_address, high_byte);
                    }
                    else
                    {
                        listing.opcode_line_with_space_2_arg(line_number, line_address,
                                                             found_opcode.code, low_byte, high_byte,
                                                             input_line);
                    }
                }
            }
            break;
            case INP_OUT: {
                int evaluated_arg1 = evaluate_argument(options, symbol_table, arguments[0]);
                const int max_port = (found_opcode.mnemonic[0] == 'i') ? 7 : 23;

                if ((evaluated_arg1 > max_port) || (evaluated_arg1 < 0))
                {
                    throw ExpectedArgumentWithinLimits(max_port, arguments[0], evaluated_arg1);
                }

                const int code = found_opcode.code + (evaluated_arg1 << 1);
                writer.write_byte(code, current_address);
                if (options.generate_list_file)
                {
                    Opcode fixed_opcode = found_opcode;
                    fixed_opcode.code = code;
                    listing.opcode_line_with_space(line_number, line_address, fixed_opcode.code,
                                                   input_line);
                }
            }
            break;
            case RST: {
                int evaluated_arg1 = evaluate_argument(options, symbol_table, arguments[0]);
                if ((evaluated_arg1 > 7) || (evaluated_arg1 < 0))
                {
                    throw ExpectedArgumentWithinLimits(7, arguments[0], evaluated_arg1);
                }

                const auto code = (found_opcode.code | (evaluated_arg1 << 3));
                writer.write_byte(code, current_address);
                if (options.generate_list_file)
                {
                    Opcode fixed_opcode = found_opcode;
                    fixed_opcode.code = code;
                    listing.opcode_line_with_space(line_number, line_address, fixed_opcode.code,
                                                   input_line);
                }
            }
            break;
            default:
                // Uncovered case.
                throw InternalError(input_line);
        }
    }
    else
    {
        // ORG, EMPTY, EQU, CPU, END
        // For END, could break here, but rather than break,
        // we will go ahead and check for more.
        if (options.generate_list_file)
        {
            listing.simple_line(line_number, input_line, options.single_byte_list);
        }
    }
}

InvalidCPU::InvalidCPU() { reason = R"(cpu only allowed is "8008" or "i8008")"; }

UnexpectedArgumentCount::UnexpectedArgumentCount(uint32_t arg_count)
{
    reason = "unexpected number of arguments: " + std::to_string(arg_count);
}
