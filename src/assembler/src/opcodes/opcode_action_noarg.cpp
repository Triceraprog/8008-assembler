#include "opcode_action_noarg.h"

#include "byte_writer.h"
#include "listing.h"

OpcodeActionNoArg::OpcodeActionNoArg(Opcode::OpcodeByteType opcode_byte, const int address)
    : opcode{opcode_byte}, address{address}
{}

void OpcodeActionNoArg::emit_byte_stream(ByteWriter& byte_writer) const
{
    byte_writer.write_byte(opcode, address);
}

void OpcodeActionNoArg::emit_listing(Listing& listing, int line_number,
                                     std::string_view input_line) const
{
    listing.opcode_line_with_space(line_number, address, opcode, input_line);
}
