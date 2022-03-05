#include "opcode_action_rst.h"

#include "byte_writer.h"
#include "evaluator.h"
#include "listing.h"

OpcodeActionRst::OpcodeActionRst(const Options& options, const SymbolTable& symbol_table,
                                 Opcode::OpcodeByteType opcode_byte, int address,
                                 const std::vector<std::string>& arguments)
    : opcode{opcode_byte}, address{address}
{
    int argument = evaluate_argument(options, symbol_table, arguments[0]);
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

void OpcodeActionRst::emit_listing(Listing& listing, int line_number, std::string_view input_line,
                                   bool single_byte) const
{
    listing.opcode_line_with_space(line_number, address, opcode, input_line);
}
