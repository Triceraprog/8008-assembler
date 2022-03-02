#ifndef INC_8008_ASSEMBLER_OPCODES_H
#define INC_8008_ASSEMBLER_OPCODES_H

#include "errors.h"

#include <string_view>
#include <tuple>

enum OpcodeType
{
    NO_ARG,
    ONE_BYTE_ARG,
    ADDRESS_ARG,
    INP_OUT,
    RST,
};

struct Opcode
{
    const char* mnemonic{};
    unsigned char code{};
    OpcodeType rule{};
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
int get_opcode_size(const Opcode& opcode);

class UndefinedOpcode : public ExceptionWithReason
{
public:
    explicit UndefinedOpcode(const std::string& opcode);
};

#endif //INC_8008_ASSEMBLER_OPCODES_H
