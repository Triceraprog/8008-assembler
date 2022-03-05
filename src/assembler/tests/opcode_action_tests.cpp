#include "opcodes/opcode_action.h"

#include "byte_writer.h"
#include "options.h"
#include "symbol_table.h"

#include "opcodes/opcode_action_inpout.h"
#include "opcodes/opcode_action_noarg.h"
#include "opcodes/opcode_action_onebyte_arg.h"
#include "opcodes/opcode_action_rst.h"
#include "opcodes/opcode_action_twobyte_arg.h"

#include "gmock/gmock.h"

using namespace testing;

struct OpcodeActionFixture : public Test
{
    Options options;
    SymbolTable symbol_table;
    Context context{options, symbol_table};

    std::stringstream byte_buffer;
    ByteWriter byte_writer{byte_buffer, ByteWriter::BINARY};

    const int current_address = 0;
};

TEST_F(OpcodeActionFixture, no_arg_action_emits_one_byte)
{
    Opcode::OpcodeByteType opcode_LAA = 0300;
    auto action = std::make_unique<OpcodeActionNoArg>(opcode_LAA, current_address);

    action->emit_byte_stream(byte_writer);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(static_cast<char>(0300)));
    ASSERT_THAT(byte_buffer.str()[1], Eq(0));
}

TEST_F(OpcodeActionFixture, one_byte_arg_action_emits_two_bytes)
{
    Opcode::OpcodeByteType opcode_LAI = 0006;
    std::vector<std::string> arguments = {"0x10"};
    auto action = std::make_unique<OpcodeActionOneByteArg>(context, opcode_LAI, current_address,
                                                           arguments);

    action->emit_byte_stream(byte_writer);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(static_cast<char>(0006)));
    ASSERT_THAT(byte_buffer.str()[1], Eq(static_cast<char>(0x10)));
    ASSERT_THAT(byte_buffer.str()[2], Eq(0));
}

TEST_F(OpcodeActionFixture, two_byte_arg_action_emits_three_bytes)
{
    Opcode::OpcodeByteType opcode_CAL = 0106;
    std::vector<std::string> arguments = {"0x1000"};
    auto action = std::make_unique<OpcodeActionTwoByteArg>(context, opcode_CAL, current_address,
                                                           arguments);

    action->emit_byte_stream(byte_writer);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(static_cast<char>(0106)));
    ASSERT_THAT(byte_buffer.str()[1], Eq(static_cast<char>(0x00)));
    ASSERT_THAT(byte_buffer.str()[2], Eq(static_cast<char>(0x10)));
    ASSERT_THAT(byte_buffer.str()[3], Eq(0));
}

TEST_F(OpcodeActionFixture, inp_out_action_emits_one_byte)
{
    Opcode::OpcodeByteType opcode_INP = 0101;
    std::vector<std::string> arguments = {"0x1"};
    auto action = std::make_unique<OpcodeActionInpOut>(context, opcode_INP, current_address,
                                                       arguments, "inp");

    action->emit_byte_stream(byte_writer);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(static_cast<char>(0103)));
    ASSERT_THAT(byte_buffer.str()[1], Eq(0));
}

TEST_F(OpcodeActionFixture, rst_action_emits_one_byte)
{
    Opcode::OpcodeByteType opcode_RST = 0005;
    std::vector<std::string> arguments = {"1"};
    auto action =
            std::make_unique<OpcodeActionRst>(context, opcode_RST, current_address, arguments);

    action->emit_byte_stream(byte_writer);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(static_cast<char>(0015)));
    ASSERT_THAT(byte_buffer.str()[1], Eq(0));
}
