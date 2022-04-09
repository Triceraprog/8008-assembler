#ifndef INC_8008_ASSEMBLER_OPCODE_ACTION_ONEBYTE_ARG_H
#define INC_8008_ASSEMBLER_OPCODE_ACTION_ONEBYTE_ARG_H

#include "opcode_action.h"
#include "context.h"

class OpcodeActionOneByteArg : public OpcodeAction
{
public:
    OpcodeActionOneByteArg(const Context& context, Opcode::OpcodeByteType opcode_byte, int address,
                           const std::vector<std::string>& arguments);

    void emit_byte_stream(ByteWriter& byte_writer) const override;
    void emit_listing(Listing& listing, std::uint32_t line_number,
                      std::string_view input_line) const override;

private:
    Opcode::OpcodeByteType opcode;
    int evaluated_argument;
    int address;
};

#endif //INC_8008_ASSEMBLER_OPCODE_ACTION_ONEBYTE_ARG_H
