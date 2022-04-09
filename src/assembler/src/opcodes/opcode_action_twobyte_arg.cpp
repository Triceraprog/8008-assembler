#include "opcode_action_twobyte_arg.h"

#include "byte_writer.h"
#include "evaluator.h"
#include "listing.h"

OpcodeActionTwoByteArg::OpcodeActionTwoByteArg(const Context& context,
                                               Opcode::OpcodeByteType opcode_byte, int address,
                                               const std::vector<std::string>& arguments)
    : opcode{opcode_byte}, address{address}
{
    const int MAX_ADDRESS = 1024 * 16;
    evaluated_argument = evaluate_argument(context, arguments[0]);
    if ((evaluated_argument > MAX_ADDRESS) || (evaluated_argument < 0))
    {
        throw ExpectedArgumentWithinLimits(MAX_ADDRESS, arguments[0], evaluated_argument);
    }
}

void OpcodeActionTwoByteArg::emit_byte_stream(ByteWriter& byte_writer) const
{
    const int low_byte = (0xFF & evaluated_argument);
    const int high_byte = (0xFF & (evaluated_argument >> 8));

    byte_writer.write_byte(opcode, address);
    byte_writer.write_byte(low_byte, address + 1);
    byte_writer.write_byte(high_byte, address + 2);
}

void OpcodeActionTwoByteArg::emit_listing(Listing& listing, std::uint32_t line_number,
                                          std::string_view input_line) const
{
    const int low_byte = (0xFF & evaluated_argument);
    const int high_byte = (0xFF & (evaluated_argument >> 8));
    if (listing.short_format())
    {
        auto line_address = address;

        listing.one_byte_of_data_with_address(line_number, line_address, opcode, input_line);
        line_address++;
        listing.one_byte_of_data_continued(line_address, low_byte);
        line_address++;
        listing.one_byte_of_data_continued(line_address, high_byte);
    }
    else
    {
        listing.opcode_line_with_space_2_arg(line_number, address, opcode, low_byte, high_byte,
                                             input_line);
    }
}
