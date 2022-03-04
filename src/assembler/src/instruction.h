#ifndef INC_8008_ASSEMBLER_INSTRUCTION_H
#define INC_8008_ASSEMBLER_INSTRUCTION_H

#include "errors.h"
#include "opcodes.h"

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

class Options;
class SymbolTable;
class ByteWriter;
class Listing;
class LineTokenizer;

enum class InstructionEnum
{
    EMPTY,
    EQU,
    END,
    CPU,
    ORG,
    DATA,
    OTHER,
};

class Instruction
{
public:
    explicit Instruction(const std::string& opcode, std::vector<std::string> arguments);

    [[nodiscard]] int get_evaluation(const Options& options, const SymbolTable& symbol_table,
                                     int current_address) const;

    [[nodiscard]] int first_pass(const Options& options, const SymbolTable& symbol_table,
                                 int current_address) const;

    void second_pass(const Options& options, const SymbolTable& symbol_table, Listing& listing,
                     ByteWriter& writer, const std::string& input_line, int line_number,
                     int address);

private:
    std::string opcode;
    InstructionEnum opcode_enum;
    const std::vector<std::string> arguments;
};

InstructionEnum instruction_to_enum(std::string_view opcode);

class InvalidCPU : public ExceptionWithReason
{
public:
    InvalidCPU();
};

#endif //INC_8008_ASSEMBLER_INSTRUCTION_H
