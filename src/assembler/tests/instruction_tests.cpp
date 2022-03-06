#include "instruction.h"

#include "byte_writer.h"
#include "context.h"
#include "listing.h"
#include "options.h"
#include "symbol_table.h"

#include "gmock/gmock.h"

using namespace testing;

struct InstructionFixture : public Test
{
    Options options;
    SymbolTable symbol_table;
    Context context{options, symbol_table};

    Instruction get_instruction_empty() const { return Instruction{context, {}, {}}; }
    Instruction get_instruction_end() const { return Instruction{context, "END", {}}; }
    Instruction get_instruction_equ() const { return Instruction{context, "EQU", {"0x2000"}}; }
    Instruction get_instruction_equ_without_param() const
    {
        return Instruction{context, "EQU", {}};
    }
    Instruction get_instruction_org() const { return Instruction{context, "ORG", {"0x1000"}}; }
    Instruction get_instruction_cpu_known() const { return Instruction{context, "CPU", {"8008"}}; }
    Instruction get_instruction_cpu_unknown() const
    {
        return Instruction{context, "CPU", {"unknown_cpu"}};
    }
    Instruction get_instruction_data() const
    {
        return Instruction{context, "DATA", {"1", "2", "3"}};
    }
    Instruction get_instruction_nop() const { return Instruction{context, "LAA", {}}; }
    Instruction get_instruction_invalid_opcode() const
    {
        return Instruction{context, "INVALID_OPCODE", {}};
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
};

/// TEST FOR PARSING THE INSTRUCTION
TEST(PseudoOpcodes, can_be_parsed_as_enum)
{
    ASSERT_THAT(instruction_to_enum(""), Eq(InstructionEnum::EMPTY));

    ASSERT_THAT(instruction_to_enum("EQU"), Eq(InstructionEnum::EQU));
    ASSERT_THAT(instruction_to_enum("equ"), Eq(InstructionEnum::EQU));

    ASSERT_THAT(instruction_to_enum("END"), Eq(InstructionEnum::END));
    ASSERT_THAT(instruction_to_enum("end"), Eq(InstructionEnum::END));

    ASSERT_THAT(instruction_to_enum("CPU"), Eq(InstructionEnum::CPU));
    ASSERT_THAT(instruction_to_enum("cpu"), Eq(InstructionEnum::CPU));

    ASSERT_THAT(instruction_to_enum("ORG"), Eq(InstructionEnum::ORG));
    ASSERT_THAT(instruction_to_enum("org"), Eq(InstructionEnum::ORG));

    ASSERT_THAT(instruction_to_enum("DATA"), Eq(InstructionEnum::DATA));
    ASSERT_THAT(instruction_to_enum("data"), Eq(InstructionEnum::DATA));

    ASSERT_THAT(instruction_to_enum("LAA"), Eq(InstructionEnum::OTHER));
    ASSERT_THAT(instruction_to_enum("garbage"), Eq(InstructionEnum::OTHER));
}

/// TESTS FOR THE ADDRESS EVALUATION

TEST_F(InstructionEvaluationFixture, returns_the_address_if_empty)
{
    auto instruction = get_instruction_empty();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(context, options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_address_if_end)
{
    auto instruction = get_instruction_end();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(context, options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_address_if_cpu)
{
    auto instruction = get_instruction_cpu_known();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(context, options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_argument_address_if_org)
{
    auto instruction = get_instruction_org();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(context, options, symbol_table, current_address),
                Eq(0x1000));
}

TEST_F(InstructionEvaluationFixture, returns_the_argument_address_if_equ)
{
    auto instruction = get_instruction_equ();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(context, options, symbol_table, current_address),
                Eq(0x2000));
}

TEST_F(InstructionEvaluationFixture, throws_if_equ_as_no_parameter)
{
    ASSERT_THROW(get_instruction_equ_without_param(), MissingArgument);
}

/// TESTS FOR THE FIRST PASS

TEST_F(FirstPassFixture, does_not_advance_address_for_empty)
{
    auto instruction = get_instruction_empty();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context, current_address), Eq(current_address));
}

TEST_F(FirstPassFixture, does_not_advance_address_for_end)
{
    auto instruction = get_instruction_end();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context, current_address), Eq(current_address));
}

TEST_F(FirstPassFixture, does_not_advance_address_for_equ)
{
    auto instruction = get_instruction_equ();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context, current_address), Eq(current_address));
}

TEST_F(FirstPassFixture, sets_address_for_org)
{
    auto instruction = get_instruction_org();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context, current_address), Eq(0x1000));
}

TEST_F(FirstPassFixture, throws_if_wrong_CPU)
{
    ASSERT_THROW(get_instruction_cpu_unknown(), InvalidCPU);
}

TEST_F(FirstPassFixture, does_not_advance_address_for_correct_cpu)
{
    auto instruction = get_instruction_cpu_known();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context, current_address), Eq(current_address));
}

TEST_F(FirstPassFixture, advance_address_with_declared_data)
{
    auto instruction = get_instruction_data();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context, current_address), Eq(current_address + 3));
}

TEST_F(FirstPassFixture, throws_if_unknown_opcode)
{
    ASSERT_THROW(get_instruction_invalid_opcode(), UndefinedOpcode);
}

TEST_F(FirstPassFixture, advances_one_byte_if_nop)
{
    auto instruction = get_instruction_nop();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context, current_address), Eq(current_address + 1));
}

/// TESTS FOR THE SECOND PASS

TEST_F(SecondPassFixture, does_not_output_byte_if_empty)
{
    auto instruction = get_instruction_empty();

    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_end)
{
    auto instruction = get_instruction_end();

    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_equ)
{
    auto instruction = get_instruction_equ();

    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_org)
{
    auto instruction = get_instruction_org();

    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, outputs_declared_data)
{
    auto instruction = get_instruction_data();

    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(1));
    ASSERT_THAT(byte_buffer.str()[1], Eq(2));
    ASSERT_THAT(byte_buffer.str()[2], Eq(3));
}

TEST_F(SecondPassFixture, outputs_opcode_data)
{
    auto instruction = get_instruction_nop();

    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(static_cast<char>(0xC0)));
    ASSERT_THAT(byte_buffer.str()[1], Eq(0));
}
