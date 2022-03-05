#include "instruction.h"
#include "byte_writer.h"
#include "data_extraction.h"
#include "evaluator.h"
#include "listing.h"
#include "opcodes/opcode_action.h"
#include "opcodes/opcodes.h"
#include "options.h"
#include "symbol_table.h"
#include "utils.h"

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

namespace
{
    void verify_cpu(const std::string& cpu_arg)
    {
        if ((!ci_equals(cpu_arg, "8008")) && (!ci_equals(cpu_arg, "i8008")))
        {
            throw InvalidCPU();
        }
    }
}

InstructionEnum instruction_to_enum(std::string_view opcode)
{
    static std::vector<std::tuple<const char*, InstructionEnum>> association = {
            {"equ", InstructionEnum::EQU},   {"end", InstructionEnum::END},
            {"cpu", InstructionEnum::CPU},   {"org", InstructionEnum::ORG},
            {"data", InstructionEnum::DATA},
    };
    if (opcode.empty())
    {
        return InstructionEnum::EMPTY;
    }
    auto found_op_code = std::ranges::find_if(association, [&opcode](const auto& t) {
        const auto& [opcode_str, opcode_enum] = t;
        return ci_equals(opcode, opcode_str);
    });

    if (found_op_code == association.end())
    {
        return InstructionEnum::OTHER;
    }

    return std::get<1>(*found_op_code);
}

Instruction::Instruction(const std::string& opcode, std::vector<std::string> arguments)
    : opcode{opcode}, arguments{std::move(arguments)}
{
    opcode_enum = instruction_to_enum(opcode);
}

int Instruction::get_evaluation(const Options& options, const SymbolTable& symbol_table,
                                int current_address) const
{
    if (opcode_enum == InstructionEnum::ORG || opcode_enum == InstructionEnum::EQU)
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
        case InstructionEnum::CPU:
            verify_cpu(arguments[0]);
        case InstructionEnum::EMPTY:
        case InstructionEnum::EQU:
        case InstructionEnum::END:
            updated_address = current_address;
            break;
        case InstructionEnum::ORG:
            updated_address = evaluate_argument(options, symbol_table, arguments[0]);
            break;

        case InstructionEnum::DATA: {
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
        case InstructionEnum::OTHER: {
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
    if (opcode_enum == InstructionEnum::DATA)
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
    else if (opcode_enum == InstructionEnum::OTHER)
    {
        const auto [found, found_opcode] = find_opcode(opcode);
        if (!found)
        {
            throw UndefinedOpcode(opcode);
        }

        auto opcode_action =
                create_opcode_action(options, symbol_table, found_opcode, address, arguments);

        opcode_action->emit_byte_stream(writer);
        if (options.generate_list_file)
        {
            opcode_action->emit_listing(listing, line_number, input_line, options.single_byte_list);
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
