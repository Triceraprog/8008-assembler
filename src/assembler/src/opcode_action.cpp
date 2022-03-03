#include "opcode_action.h"

#include "byte_writer.h"
#include "evaluator.h"
#include "listing.h"

OpcodeActionNoArg::OpcodeActionNoArg(Opcode::OpcodeByteType opcode_byte, const int address)
    : opcode{opcode_byte}, address{address}
{}

void OpcodeActionNoArg::emit_byte_stream(ByteWriter& byte_writer) const
{
    byte_writer.write_byte(opcode, address);
}

void OpcodeActionNoArg::emit_listing(Listing& listing, int line_number, std::string_view input_line,
                                     bool single_byte) const
{
    listing.opcode_line_with_space(line_number, address, opcode, input_line);
}

OpcodeActionOneByteArg::OpcodeActionOneByteArg(const Options& options,
                                               const SymbolTable& symbol_table,
                                               Opcode::OpcodeByteType opcode_byte, int address,
                                               const std::vector<std::string>& arguments)
    : opcode{opcode_byte}, address{address}
{
    evaluated_argument = evaluate_argument(options, symbol_table, arguments[0]);
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

OpcodeActionTwoByteArg::OpcodeActionTwoByteArg(const Options& options,
                                               const SymbolTable& symbol_table,
                                               Opcode::OpcodeByteType opcode_byte, int address,
                                               const std::vector<std::string>& arguments)
    : opcode{opcode_byte}, address{address}
{
    const int MAX_ADDRESS = 1024 * 16;
    evaluated_argument = evaluate_argument(options, symbol_table, arguments[0]);
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

void OpcodeActionTwoByteArg::emit_listing(Listing& listing, int line_number,
                                          std::string_view input_line, bool single_byte) const
{
    const int low_byte = (0xFF & evaluated_argument);
    const int high_byte = (0xFF & (evaluated_argument >> 8));
    if (single_byte)
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

std::unique_ptr<OpcodeAction> create_opcode_action(const Options& options,
                                                   const SymbolTable& symbol_table, Opcode opcode,
                                                   int address,
                                                   const std::vector<std::string>& arguments)
{
    switch (opcode.rule)
    {
        case NO_ARG:
            return std::make_unique<OpcodeActionNoArg>(opcode.code, address);
        case ONE_BYTE_ARG:
            return std::make_unique<OpcodeActionOneByteArg>(options, symbol_table, opcode.code,
                                                            address, arguments);
        case ADDRESS_ARG:
            return std::make_unique<OpcodeActionTwoByteArg>(options, symbol_table, opcode.code,
                                                            address, arguments);
        case INP_OUT:
            return std::make_unique<OpcodeActionInpOut>(options, symbol_table, opcode.code, address,
                                                        arguments, opcode.mnemonic);
        case RST:
            return std::make_unique<OpcodeActionRst>(options, symbol_table, opcode.code, address,
                                                     arguments);
    }
    return nullptr;
}

ExpectedArgumentWithinLimits::ExpectedArgumentWithinLimits(int limit, const std::string& content,
                                                           int evaluated)
{
    reason = "expected argument between 0 and " + std::to_string(limit) + " instead got " +
             content + "=" + std::to_string(evaluated);
}
