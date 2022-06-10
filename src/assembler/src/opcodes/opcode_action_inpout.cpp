#include "opcode_action_inpout.h"

#include "byte_writer.h"
#include "listing.h"

OpcodeActionInpOut::OpcodeActionInpOut(const Context& context, Opcode::OpcodeByteType opcode_byte,
                                       int address, const std::vector<std::string>& arguments,
                                       std::string_view mnemonic)
    : address{address}
{
    int argument = evaluate(context, arguments[0]);
    const bool is_input = mnemonic[0] == 'i';
    const int max_port = is_input ? 7 : 23;

    if ((argument > max_port) || (argument < 0))
    {
        throw ExpectedArgumentWithinLimits(max_port, arguments[0], argument);
    }

    opcode = opcode_byte + (argument << 1) + (is_input ? 0 : 16);
}

void OpcodeActionInpOut::emit_byte_stream(ByteWriter& byte_writer) const
{
    byte_writer.write_byte(opcode, address);
}

void OpcodeActionInpOut::emit_listing(Listing& listing, std::uint32_t line_number,
                                      std::string_view input_line) const
{
    listing.opcode_line_with_space(line_number, address, opcode, input_line);
}
