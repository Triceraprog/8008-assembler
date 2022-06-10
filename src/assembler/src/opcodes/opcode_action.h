#ifndef INC_8008_ASSEMBLER_OPCODE_ACTION_H
#define INC_8008_ASSEMBLER_OPCODE_ACTION_H

#include "opcodes.h"

#include "context.h"
#include "options.h"
#include "symbol_table.h"

#include <memory>
#include <string>
#include <vector>

class ByteWriter;
class Listing;

class OpcodeAction
{
public:
    virtual void emit_byte_stream(ByteWriter& byte_writer) const = 0;
    virtual void emit_listing(Listing& listing, std::uint32_t line_number,
                              std::string_view input_line) const = 0;

    virtual ~OpcodeAction() = default;

    static int evaluate(const Context& context, std::string_view arg);
};

std::unique_ptr<OpcodeAction> create_opcode_action(const Context& context, Opcode opcode,
                                                   int address,
                                                   const std::vector<std::string>& arguments);

class ExpectedArgumentWithinLimits : public ExceptionWithReason
{
public:
    ExpectedArgumentWithinLimits(int limit, const std::string& content, int evaluated);
};

class UnexpectedArgumentCount : public ExceptionWithReason
{
public:
    explicit UnexpectedArgumentCount(uint32_t arg_count);
};

#endif //INC_8008_ASSEMBLER_OPCODE_ACTION_H
