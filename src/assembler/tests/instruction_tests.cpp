#include "instruction.h"

#include "byte_writer.h"
#include "listing.h"
#include "options.h"
#include "symbol_table.h"

#include "gmock/gmock.h"

using namespace testing;

struct InstructionFixture : public Test
{
    Options options;
    SymbolTable symbol_table;
};

struct InstructionEvaluationFixture : public InstructionFixture
{
};

struct FirstPassFixture : public InstructionFixture
{
};

struct SecondPassFixture : public InstructionFixture
{
    std::stringstream byte_buffer;
    std::stringstream listing_buffer;
    ByteWriter byte_writer{byte_buffer, ByteWriter::BINARY};
    Listing listing{listing_buffer, options};
};

TEST_F(InstructionEvaluationFixture, returns_the_address_if_empty)
{
    std::string opcode{};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_address_if_end)
{
    std::string opcode{"END"};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_address_if_cpu)
{
    std::string opcode{"CPU"};
    std::vector<std::string> arguments{"8008"};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_argument_address_if_org)
{
    std::string opcode{"ORG"};
    std::vector<std::string> arguments{"0x1000"};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address), Eq(0x1000));
}

TEST_F(InstructionEvaluationFixture, returns_the_argument_address_if_equ)
{
    std::string opcode{"EQU"};
    std::vector<std::string> arguments{"0x2000"};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address), Eq(0x2000));
}

TEST_F(FirstPassFixture, does_not_advance_address_for_empty)
{
    std::string opcode{};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(FirstPassFixture, does_not_advance_address_for_end)
{
    std::string opcode{"END"};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(FirstPassFixture, does_not_advance_address_for_equ)
{
    std::string opcode{"EQU"};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(FirstPassFixture, sets_address_for_org)
{
    std::string opcode{"ORG"};
    std::vector<std::string> arguments{"0x1000"};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address), Eq(0x1000));
}

TEST_F(FirstPassFixture, throws_if_wrong_CPU)
{
    std::string opcode{"CPU"};
    std::vector<std::string> arguments{"unknown_cpu"};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    int return_value = 0;
    ASSERT_THROW(return_value = instruction.first_pass(options, symbol_table, current_address),
                 InvalidCPU);
    ASSERT_THAT(return_value, Eq(0));
}

TEST_F(FirstPassFixture, does_not_advance_address_for_correct_cpu)
{
    std::string opcode{"CPU"};
    std::vector<std::string> arguments{"8008"};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(FirstPassFixture, advance_address_with_declared_data)
{
    std::string opcode{"DATA"};
    std::vector<std::string> arguments{"1", "2", "3"};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address + 3));
}

TEST_F(FirstPassFixture, throws_if_unknown_opcode)
{
    std::string opcode{"INVALID_OPCODE"};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    int return_value = 0;
    ASSERT_THROW(return_value = instruction.first_pass(options, symbol_table, current_address),
                 UndefinedOpcode);
    ASSERT_THAT(return_value, Eq(0));
}

TEST_F(FirstPassFixture, advances_one_byte_if_nop)
{
    std::string opcode{"LAA"};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address + 1));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_empty)
{
    std::string opcode{};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    const int current_address = 0;
    const int line_number = 1000;
    instruction.second_pass(options, symbol_table, listing, byte_writer, "", line_number,
                            current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_end)
{
    std::string opcode{"END"};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    const int current_address = 0;
    const int line_number = 1000;
    instruction.second_pass(options, symbol_table, listing, byte_writer, "", line_number,
                            current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_equ)
{
    std::string opcode{"EQU"};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    const int current_address = 0;
    const int line_number = 1000;
    instruction.second_pass(options, symbol_table, listing, byte_writer, "", line_number,
                            current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_org)
{
    std::string opcode{"ORG"};
    std::vector<std::string> arguments{"0x1000"};

    Instruction instruction{opcode, arguments};

    const int current_address = 0;
    const int line_number = 1000;
    instruction.second_pass(options, symbol_table, listing, byte_writer, "", line_number,
                            current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, outputs_declared_data)
{
    std::string opcode{"DATA"};
    std::vector<std::string> arguments{"1", "2", "3"};

    Instruction instruction{opcode, arguments};

    const int current_address = 0;
    const int line_number = 1000;
    instruction.second_pass(options, symbol_table, listing, byte_writer, "", line_number,
                            current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(1));
    ASSERT_THAT(byte_buffer.str()[1], Eq(2));
    ASSERT_THAT(byte_buffer.str()[2], Eq(3));
}
