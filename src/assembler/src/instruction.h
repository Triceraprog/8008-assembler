#ifndef INC_8008_ASSEMBLER_INSTRUCTION_H
#define INC_8008_ASSEMBLER_INSTRUCTION_H

#include <string>
#include <vector>

#include "opcodes.h"

class Options;
class SymbolTable;

class Instruction
{
public:
    explicit Instruction(const std::string& opcode, const std::vector<std::string>& arguments);

    [[nodiscard]] int get_evaluation(const Options& options, SymbolTable& symbol_table,
                                     int current_address) const;

private:
    PseudoOpcodeEnum opcode_enum;
    const std::vector<std::string>& arguments;
};

#endif //INC_8008_ASSEMBLER_INSTRUCTION_H
