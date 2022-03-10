#ifndef INC_8008_ASSEMBLER_OPCODES_H
#define INC_8008_ASSEMBLER_OPCODES_H

#include "errors.h"

#include <span>
#include <string>
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
    using OpcodeByteType = unsigned char;
    const char* mnemonic{};
    OpcodeByteType code{};
    OpcodeType rule{};
};

std::tuple<bool, Opcode, std::size_t> find_opcode(std::string_view opcode_name,
                                                  std::span<std::string> arguments);
int get_opcode_size(const Opcode& opcode);

class UndefinedOpcode : public ExceptionWithReason
{
public:
    explicit UndefinedOpcode(std::string_view opcode);
};

class SyntaxError : public ExceptionWithReason
{
public:
    explicit SyntaxError(std::string_view reason);
};

#endif //INC_8008_ASSEMBLER_OPCODES_H
