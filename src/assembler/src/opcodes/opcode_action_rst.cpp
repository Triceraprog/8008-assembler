#include "opcode_action_rst.h"

#include "byte_writer.h"
#include "evaluator.h"
#include "listing.h"

OpcodeActionRst::OpcodeActionRst(const Context& context, Opcode::OpcodeByteType opcode_byte,
                                 int address, const std::vector<std::string>& arguments)
    : opcode{opcode_byte}, address{address}
{
    int argument = evaluate_argument(context, arguments[0]);
    if (argument % 8 == 0)
    {
        argument /= 8;
    }
    if ((argument > 7) || (argument < 0))
    {
        throw ExpectedArgumentWithinLimits(7, arguments[0], argument);
    }

    this->opcode = (opcode | (argument << 3));
}

void OpcodeActionRst::emit_byte_stream(ByteWriter& byte_writer) const
{
    byte_writer.write_byte(opcode, address);
}

void OpcodeActionRst::emit_listing(Listing& listing, std::uint32_t line_number,
                                   std::string_view input_line) const
{
    listing.opcode_line_with_space(line_number, address, opcode, input_line);
}
