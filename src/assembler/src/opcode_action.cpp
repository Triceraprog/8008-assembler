#include "opcode_action.h"

#include "byte_writer.h"
#include "evaluator.h"
#include "instruction.h"

OpcodeActionNoArg::OpcodeActionNoArg(unsigned char opcode, const int address)
    : opcode{opcode}, address{address}
{}

void OpcodeActionNoArg::emit_byte_stream(ByteWriter& byte_writer) const
{
    byte_writer.write_byte(opcode, address);
}

OpcodeActionOneByteArg::OpcodeActionOneByteArg(const Options& options,
                                               const SymbolTable& symbol_table,
                                               unsigned char opcode, int address,
                                               const std::vector<std::string>& arguments)
    : opcode{opcode}, address{address}
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

OpcodeActionTwoByteArg::OpcodeActionTwoByteArg(const Options& options,
                                               const SymbolTable& symbol_table,
                                               unsigned char opcode, int address,
                                               const std::vector<std::string>& arguments)
    : opcode{opcode}, address{address}
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

OpcodeActionInpOut::OpcodeActionInpOut(const Options& options, const SymbolTable& symbol_table,
                                       unsigned char opcode, int address,
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

    this->opcode = opcode + (argument << 1);
}

void OpcodeActionInpOut::emit_byte_stream(ByteWriter& byte_writer) const
{
    byte_writer.write_byte(opcode, address);
}

OpcodeActionRst::OpcodeActionRst(const Options& options, const SymbolTable& symbol_table,
                                 unsigned char opcode, int address,
                                 const std::vector<std::string>& arguments)
    : opcode{opcode}, address{address}
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
