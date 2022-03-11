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

enum SyntaxType
{
    OLD,
    NEW
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

using matcher_signature = std::tuple<bool, Opcode, std::size_t>(std::string_view opcode_name,
                                                                std::span<std::string> arguments);

matcher_signature* get_opcode_matcher(SyntaxType syntax_type);

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
