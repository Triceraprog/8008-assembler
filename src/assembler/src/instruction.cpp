#include "instruction.h"
#include "data_extraction.h"
#include "evaluator.h"
#include "options.h"
#include "symbol_table.h"
#include "utils.h"
#include <iostream>

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

Instruction::Instruction(const std::string& opcode, const std::vector<std::string>& arguments)
    : opcode{opcode}, arguments{arguments}
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

InvalidCPU::InvalidCPU() { reason = R"(cpu only allowed is "8008" or "i8008")"; }
