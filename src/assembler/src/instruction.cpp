#include "instruction.h"
#include "byte_writer.h"
#include "context.h"
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

    struct Instruction_EQU : public Instruction::InstructionAction
    {
        explicit Instruction_EQU(const std::vector<std::string>& arguments)
            : first_arg{arguments[0]}
        {}
        [[nodiscard]] int evaluate_fixed_address(const Context& context,
                                                 int current_address) const override
        {
            return evaluate_argument(context, first_arg);
        }

        std::string first_arg;
    };

    struct Instruction_END : public Instruction::InstructionAction
    {
    };

    struct Instruction_CPU : public Instruction::InstructionAction
    {
        explicit Instruction_CPU(const std::vector<std::string>& arguments)
        {
            verify_cpu(arguments[0]);
        }
    };

    struct Instruction_ORG : public Instruction::InstructionAction
    {
        explicit Instruction_ORG(const std::vector<std::string>& arguments)
            : first_arg{arguments[0]}
        {}
        [[nodiscard]] int evaluate_fixed_address(const Context& context,
                                                 int current_address) const override
        {
            return evaluate_argument(context, first_arg);
        }
        [[nodiscard]] int advance_address(const Context& context,
                                          int current_address) const override
        {
            return evaluate_argument(context, first_arg);
        }

        std::string first_arg;
    };

    struct Instruction_DATA : public Instruction::InstructionAction
    {
        explicit Instruction_DATA(std::vector<std::string> arguments)
            : arguments{std::move(arguments)}
        {}

        [[nodiscard]] int advance_address(const Context& context,
                                          int current_address) const override
        {
            std::vector<int> data_list;
            int data_size = decode_data(context, arguments, data_list);

            if (context.options.debug)
            {
                std::cout << "got " << data_size << " items in data list\n";
            }

            /* a negative number denotes that much space to save, but not specifying data */
            return current_address + std::abs(data_size);
        }

        std::vector<std::string> arguments;
    };

    struct Instruction_OTHER : public Instruction::InstructionAction
    {
        explicit Instruction_OTHER(std::string_view opcode) : opcode{opcode} {}

        [[nodiscard]] int advance_address(const Context& context,
                                          int current_address) const override
        {
            if (auto [found, found_opcode] = find_opcode(opcode); found)
            {
                return current_address + get_opcode_size(found_opcode);
            }
            throw UndefinedOpcode(opcode);
        }

        std::string opcode;
    };

    struct Instruction_EMPTY : public Instruction::InstructionAction
    {
    };

}

int Instruction::InstructionAction::evaluate_fixed_address(const Context& context,
                                                           int current_address) const
{
    return current_address;
}

int Instruction::InstructionAction::advance_address(const Context& context,
                                                    int current_address) const
{
    return current_address;
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

    switch (opcode_enum)
    {
        case InstructionEnum::CPU:
            action = std::make_unique<Instruction_CPU>(this->arguments);
            break;
        case InstructionEnum::EMPTY:
            action = std::make_unique<Instruction_EMPTY>();
            break;
        case InstructionEnum::EQU:
            action = std::make_unique<Instruction_EQU>(this->arguments);
            break;
        case InstructionEnum::END:
            action = std::make_unique<Instruction_END>();
            break;
        case InstructionEnum::ORG:
            action = std::make_unique<Instruction_ORG>(this->arguments);
            break;
        case InstructionEnum::DATA:
            action = std::make_unique<Instruction_DATA>(this->arguments);
            break;
        case InstructionEnum::OTHER:
            action = std::make_unique<Instruction_OTHER>(opcode);
            break;
    }
}

int Instruction::get_evaluation(const Context& context, const Options& options,
                                const SymbolTable& symbol_table, int current_address) const
{
    return action->evaluate_fixed_address(context, current_address);
}

int Instruction::first_pass(const Context& context, int current_address) const
{
    return action->advance_address(context, current_address);
}

void Instruction::second_pass(const Context& context, Listing& listing, ByteWriter& writer,
                              const std::string& input_line, int line_number,
                              const int address) const
{
    const bool generate_list_file = context.options.generate_list_file;
    if (opcode_enum == InstructionEnum::DATA)
    {
        std::vector<int> data_list;
        const int data_length = decode_data(context, arguments, data_list);
        if (data_length < 0)
        {
            /* if n is negative, that number of bytes are just reserved */
            if (generate_list_file)
            {
                listing.reserved_data(line_number, address, input_line,
                                      context.options.single_byte_list);
            }
        }
        else
        {
            for (int write_address = address; const auto& data : data_list)
            {
                writer.write_byte(data, write_address);
                write_address += 1;
            }
            if (generate_list_file)
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

        auto opcode_action = create_opcode_action(context, found_opcode, address, arguments);

        opcode_action->emit_byte_stream(writer);
        if (generate_list_file)
        {
            opcode_action->emit_listing(listing, line_number, input_line,
                                        context.options.single_byte_list);
        }
    }
    else
    {
        // ORG, EMPTY, EQU, CPU, END
        // For END, could break here, but rather than break,
        // we will go ahead and check for more.
        if (generate_list_file)
        {
            listing.simple_line(line_number, input_line, context.options.single_byte_list);
        }
    }
}

InvalidCPU::InvalidCPU() { reason = R"(cpu only allowed is "8008" or "i8008")"; }
