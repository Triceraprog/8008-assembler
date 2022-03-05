#include "opcode_action_inpout.h"

#include "byte_writer.h"
#include "evaluator.h"
#include "listing.h"

OpcodeActionInpOut::OpcodeActionInpOut(const Options& options, const SymbolTable& symbol_table,
                                       Opcode::OpcodeByteType opcode_byte, int address,
                                       const std::vector<std::string>& arguments,
                                       std::string_view mnemonic)
    : address{address}
{
    int argument = evaluate_argument(options, symbol_table, arguments[0]);
    const int max_port = (mnemonic[0] == 'i') ? 7 : 23;

    if ((argument > max_port) || (argument < 0))
    {
        throw ExpectedArgumentWithinLimits(max_port, arguments[0], argument);
    }

    opcode = opcode_byte + (argument << 1);
}

void OpcodeActionInpOut::emit_byte_stream(ByteWriter& byte_writer) const
{
    byte_writer.write_byte(opcode, address);
}

void OpcodeActionInpOut::emit_listing(Listing& listing, int line_number,
                                      std::string_view input_line, bool single_byte) const
{
    listing.opcode_line_with_space(line_number, address, opcode, input_line);
}
