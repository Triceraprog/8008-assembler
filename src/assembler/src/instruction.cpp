#include "instruction.h"
#include "evaluator.h"
#include "options.h"
#include "symbol_table.h"

Instruction::Instruction(const std::string& opcode, const std::vector<std::string>& arguments)
    : arguments{arguments}
{
    opcode_enum = opcode_to_enum(opcode);
}

int Instruction::get_evaluation(const Options& options, SymbolTable& symbol_table,
                                int current_address) const
{
    if (opcode_enum == PseudoOpcodeEnum::ORG || opcode_enum == PseudoOpcodeEnum::EQU)
    {
        return evaluate_argument(options, symbol_table, arguments[0]);
    }
    return current_address;
}
