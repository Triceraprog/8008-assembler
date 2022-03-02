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

    static Instruction get_instruction_empty() { return Instruction{{}, {}}; }
    static Instruction get_instruction_end() { return Instruction{"END", {}}; }
    static Instruction get_instruction_equ() { return Instruction{"EQU", {"0x2000"}}; }
    static Instruction get_instruction_org() { return Instruction{"ORG", {"0x1000"}}; }
    static Instruction get_instruction_cpu_known() { return Instruction{"CPU", {"8008"}}; }
    static Instruction get_instruction_cpu_unknown() { return Instruction{"CPU", {"unknown_cpu"}}; }
    static Instruction get_instruction_data() { return Instruction{"DATA", {"1", "2", "3"}}; }
    static Instruction get_instruction_nop() { return Instruction{"LAA", {}}; }
    static Instruction get_instruction_invalid_opcode()
    {
        return Instruction{"INVALID_OPCODE", {}};
    }
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

    const int current_address = 0;
    const int line_number = 1000;
};

/// TESTS FOR THE ADDRESS EVALUATION

TEST_F(InstructionEvaluationFixture, returns_the_address_if_empty)
{
    auto instruction = get_instruction_empty();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_address_if_end)
{
    auto instruction = get_instruction_end();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_address_if_cpu)
{
    auto instruction = get_instruction_cpu_known();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_argument_address_if_org)
{
    auto instruction = get_instruction_org();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address), Eq(0x1000));
}

TEST_F(InstructionEvaluationFixture, returns_the_argument_address_if_equ)
{
    auto instruction = get_instruction_equ();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address), Eq(0x2000));
}

/// TESTS FOR THE FIRST PASS

TEST_F(FirstPassFixture, does_not_advance_address_for_empty)
{
    auto instruction = get_instruction_empty();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(FirstPassFixture, does_not_advance_address_for_end)
{
    auto instruction = get_instruction_end();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(FirstPassFixture, does_not_advance_address_for_equ)
{
    auto instruction = get_instruction_equ();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(FirstPassFixture, sets_address_for_org)
{
    auto instruction = get_instruction_org();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address), Eq(0x1000));
}

TEST_F(FirstPassFixture, throws_if_wrong_CPU)
{
    auto instruction = get_instruction_cpu_unknown();

    const int current_address = 0xff;
    int return_value = 0;
    ASSERT_THROW(return_value = instruction.first_pass(options, symbol_table, current_address),
                 InvalidCPU);
    ASSERT_THAT(return_value, Eq(0));
}

TEST_F(FirstPassFixture, does_not_advance_address_for_correct_cpu)
{
    auto instruction = get_instruction_cpu_known();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(FirstPassFixture, advance_address_with_declared_data)
{
    auto instruction = get_instruction_data();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address + 3));
}

TEST_F(FirstPassFixture, throws_if_unknown_opcode)
{
    auto instruction = get_instruction_invalid_opcode();

    const int current_address = 0xff;
    int return_value = 0;
    ASSERT_THROW(return_value = instruction.first_pass(options, symbol_table, current_address),
                 UndefinedOpcode);
    ASSERT_THAT(return_value, Eq(0));
}

TEST_F(FirstPassFixture, advances_one_byte_if_nop)
{
    auto instruction = get_instruction_nop();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(options, symbol_table, current_address),
                Eq(current_address + 1));
}

/// TESTS FOR THE SECOND PASS

TEST_F(SecondPassFixture, does_not_output_byte_if_empty)
{
    auto instruction = get_instruction_empty();

    instruction.second_pass(options, symbol_table, listing, byte_writer, "", line_number,
                            current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_end)
{
    auto instruction = get_instruction_end();

    instruction.second_pass(options, symbol_table, listing, byte_writer, "", line_number,
                            current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_equ)
{
    auto instruction = get_instruction_equ();

    instruction.second_pass(options, symbol_table, listing, byte_writer, "", line_number,
                            current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_org)
{
    auto instruction = get_instruction_org();

    instruction.second_pass(options, symbol_table, listing, byte_writer, "", line_number,
                            current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, outputs_declared_data)
{
    auto instruction = get_instruction_data();

    instruction.second_pass(options, symbol_table, listing, byte_writer, "", line_number,
                            current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(1));
    ASSERT_THAT(byte_buffer.str()[1], Eq(2));
    ASSERT_THAT(byte_buffer.str()[2], Eq(3));
}

TEST_F(SecondPassFixture, outputs_opcode_data)
{
    auto instruction = get_instruction_nop();

    instruction.second_pass(options, symbol_table, listing, byte_writer, "", line_number,
                            current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(static_cast<char>(0xC0)));
    ASSERT_THAT(byte_buffer.str()[1], Eq(0));
}
