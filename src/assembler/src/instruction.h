#ifndef INC_8008_ASSEMBLER_INSTRUCTION_H
#define INC_8008_ASSEMBLER_INSTRUCTION_H

#include <string>
#include <vector>

#include "opcodes.h"

class Options;
class SymbolTable;
class ByteWriter;
class Listing;
class LineTokenizer;

class Instruction
{
public:
    explicit Instruction(const std::string& opcode, const std::vector<std::string>& arguments);

    [[nodiscard]] int get_evaluation(const Options& options, const SymbolTable& symbol_table,
                                     int current_address) const;

    [[nodiscard]] int first_pass(const Options& options, const SymbolTable& symbol_table,
                                 int current_address) const;

    void second_pass(const Options& options, const SymbolTable& symbol_table, Listing& listing,
                     ByteWriter& writer, const std::string& input_line, int line_number,
                     int address);

private:
    std::string opcode;
    PseudoOpcodeEnum opcode_enum;
    const std::vector<std::string>& arguments;
};

class InvalidCPU : public ExceptionWithReason
{
public:
    InvalidCPU();
};

#endif //INC_8008_ASSEMBLER_INSTRUCTION_H
