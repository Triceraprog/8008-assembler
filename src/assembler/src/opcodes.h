#ifndef INC_8008_ASSEMBLER_OPCODES_H
#define INC_8008_ASSEMBLER_OPCODES_H

#include "errors.h"

#include <string_view>
#include <tuple>

struct Opcode
{
    const char* mnemonic{};
    unsigned char code{};
    int rule{};
};

enum class PseudoOpcodeEnum
{
    EMPTY,
    EQU,
    END,
    CPU,
    ORG,
    DATA,
    OTHER,
};

std::tuple<bool, Opcode&> find_opcode(std::string_view opcode_name);
PseudoOpcodeEnum opcode_to_enum(std::string_view opcode);

class UndefinedOpcode : public ExceptionWithReason
{
public:
    UndefinedOpcode(const std::string& opcode);
};

#endif //INC_8008_ASSEMBLER_OPCODES_H
