#include "opcode_action_onebyte_arg.h"

#include "byte_writer.h"
#include "evaluator.h"
#include "listing.h"

OpcodeActionOneByteArg::OpcodeActionOneByteArg(const Context& context,
                                               Opcode::OpcodeByteType opcode_byte, int address,
                                               const std::vector<std::string>& arguments)
    : opcode{opcode_byte}, address{address}
{
    evaluated_argument = evaluate_argument(context, arguments[0]);
    if ((evaluated_argument > 255) || (evaluated_argument < 0))
    {
        throw ExpectedArgumentWithinLimits(255, arguments[0], evaluated_argument);
    }
}

void OpcodeActionOneByteArg::emit_byte_stream(ByteWriter& byte_writer) const
{
    byte_writer.write_byte(opcode, address);
    byte_writer.write_byte(evaluated_argument, address + 1);
}

void OpcodeActionOneByteArg::emit_listing(Listing& listing, int line_number,
                                          std::string_view input_line, bool single_byte) const
{
    if (single_byte)
    {
        auto line_address = address;
        listing.one_byte_of_data_with_address(line_number, line_address, opcode, input_line);
        line_address++;
        listing.one_byte_of_data_continued(line_address, evaluated_argument);
    }
    else
    {

        listing.opcode_line_with_space_1_arg(line_number, address, opcode, evaluated_argument,
                                             input_line);
    }
}
