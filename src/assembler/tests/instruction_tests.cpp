#include "instruction.h"

#include "byte_writer.h"
#include "context.h"
#include "context_stack.h"
#include "files/file_reader.h"
#include "listing.h"
#include "options.h"

#include "gmock/gmock.h"

using namespace testing;

struct InstructionFixture : public Test
{
    Options options;
    ContextStack context_stack{options};
    FileReader file_reader;

    Instruction get_instruction_empty()
    {
        return Instruction{*context_stack.get_current_context(), "", {}, {}, file_reader};
    }
    Instruction get_instruction_end()
    {
        return Instruction{*context_stack.get_current_context(), "", "END", {}, file_reader};
    }
    Instruction get_instruction_equ()
    {
        return Instruction{*context_stack.get_current_context(),
                           "",
                           "EQU",
                           {"0x2000"},
                           file_reader};
    }
    Instruction get_instruction_equ_without_param()
    {
        return Instruction{*context_stack.get_current_context(), "", "EQU", {}, file_reader};
    }
    Instruction get_instruction_org()
    {
        return Instruction{*context_stack.get_current_context(),
                           "",
                           "ORG",
                           {"0x1000"},
                           file_reader};
    }
    Instruction get_instruction_org_without_param()
    {
        return Instruction{*context_stack.get_current_context(), "", "ORG", {}, file_reader};
    }
    Instruction get_instruction_cpu_known()
    {
        return Instruction{*context_stack.get_current_context(), "", "CPU", {"8008"}, file_reader};
    }
    Instruction get_instruction_cpu_unknown()
    {
        return Instruction{*context_stack.get_current_context(),
                           "",
                           "CPU",
                           {"unknown_cpu"},
                           file_reader};
    }
    Instruction get_instruction_cpu_without_param()
    {
        return Instruction{*context_stack.get_current_context(), "", "CPU", {}, file_reader};
    }
    Instruction get_instruction_data()
    {
        return Instruction{*context_stack.get_current_context(),
                           "",
                           "DATA",
                           {"1", "2", "3"},
                           file_reader};
    }
    Instruction get_instruction_syntax()
    {
        return Instruction{*context_stack.get_current_context(),
                           "",
                           ".SYNTAX",
                           {"OLD"},
                           file_reader};
    }
    Instruction get_instruction_syntax_without_param()
    {
        return Instruction{*context_stack.get_current_context(), "", ".SYNTAX", {}, file_reader};
    }
    Instruction get_instruction_context()
    {
        return Instruction{*context_stack.get_current_context(),
                           "",
                           ".CONTEXT",
                           {"PUSH"},
                           file_reader};
    }
    Instruction get_instruction_context_without_param()
    {
        return Instruction{*context_stack.get_current_context(), "", ".CONTEXT", {}, file_reader};
    }
    Instruction get_instruction_nop()
    {
        return Instruction{*context_stack.get_current_context(), "", "LAA", {}, file_reader};
    }
    Instruction get_instruction_invalid_opcode()
    {
        return Instruction{*context_stack.get_current_context(),
                           "",
                           "INVALID_OPCODE",
                           {},
                           file_reader};
    }

    Instruction get_instruction_if_false()
    {
        return Instruction{*context_stack.get_current_context(), "", ".IF", {"0"}, file_reader};
    }

    Instruction get_instruction_if_true()
    {
        return Instruction{*context_stack.get_current_context(), "", ".IF", {"1"}, file_reader};
    }

    Instruction get_instruction_if_without_param()
    {
        return Instruction{*context_stack.get_current_context(), "", ".IF", {}, file_reader};
    }

    Instruction get_instruction_else()
    {
        return Instruction{*context_stack.get_current_context(), "", ".ELSE", {}, file_reader};
    }
    Instruction get_instruction_endif()
    {
        return Instruction{*context_stack.get_current_context(), "", ".ENDIF", {}, file_reader};
    }

    Instruction get_instruction_macro()
    {
        return Instruction{*context_stack.get_current_context(), "", ".MACRO", {}, file_reader};
    }
    Instruction get_instruction_endmacro()
    {
        return Instruction{*context_stack.get_current_context(), "", ".ENDMACRO", {}, file_reader};
    }

    Instruction get_macro_call()
    {
        return Instruction{*context_stack.get_current_context(), "", ".a_macro", {}, file_reader};
    }
};

struct InstructionEvaluationFixture : public InstructionFixture
{
};

struct InstructionEvaluationFixtureNewSyntax : public InstructionFixture
{
    InstructionEvaluationFixtureNewSyntax()
    {
        context_stack.get_current_context()->get_options().new_syntax = true;
    }

    Instruction get_instruction_mvi()
    {
        return Instruction{*context_stack.get_current_context(),
                           "",
                           "MVI",
                           {"A", "0x42"},
                           file_reader};
    }
    Instruction get_instruction_add()
    {
        return Instruction{*context_stack.get_current_context(), "", "ADD", {"A"}, file_reader};
    }
};

struct FirstPassFixture : public InstructionFixture
{
    void context_pop_and_verify()
    {
        // Place a symbol on the current context.
        // Then pops the context.
        // If the symbol is not found, we can say the context was really popped.
        context_stack.get_current_context()->define_symbol("CONTEXT_MARKER", 12345);
        context_stack.pop();
        const auto [result, value] =
                context_stack.get_current_context()->get_symbol_value("CONTEXT_MARKER");
        ASSERT_THAT(result, IsFalse());
    }
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
    auto& context = *context_stack.get_current_context();
    ASSERT_THAT(instruction.get_value_for_label(context, current_address), Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_address_if_end)
{
    auto instruction = get_instruction_end();

    const int current_address = 0xff;
    auto& context = *context_stack.get_current_context();
    ASSERT_THAT(instruction.get_value_for_label(context, current_address), Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_address_if_cpu)
{
    auto instruction = get_instruction_cpu_known();

    const int current_address = 0xff;
    auto& context = *context_stack.get_current_context();

    ASSERT_THAT(instruction.get_value_for_label(context, current_address), Eq(current_address));
}

TEST_F(InstructionEvaluationFixture, returns_the_argument_address_if_org)
{
    auto instruction = get_instruction_org();

    const int current_address = 0xff;
    auto& context = *context_stack.get_current_context();
    ASSERT_THAT(instruction.get_value_for_label(context, current_address), Eq(0x1000));
}

TEST_F(InstructionEvaluationFixture, returns_the_argument_address_if_equ)
{
    auto instruction = get_instruction_equ();

    const int current_address = 0xff;
    auto& context = *context_stack.get_current_context();
    ASSERT_THAT(instruction.get_value_for_label(context, current_address), Eq(0x2000));
}

/// JUST CHECK SYNTAX

TEST_F(InstructionEvaluationFixture, throws_if_equ_as_no_parameter)
{
    ASSERT_THROW(get_instruction_equ_without_param(), MissingArgument);
}

TEST_F(InstructionEvaluationFixture, throws_if_org_as_no_parameter)
{
    ASSERT_THROW(get_instruction_org_without_param(), MissingArgument);
}

TEST_F(InstructionEvaluationFixture, throws_if_cpu_as_no_parameter)
{
    ASSERT_THROW(get_instruction_cpu_without_param(), MissingArgument);
}

TEST_F(InstructionEvaluationFixtureNewSyntax, decodes_new_syntax_mvi)
{
    ASSERT_NO_THROW(get_instruction_mvi());
}

TEST_F(InstructionEvaluationFixtureNewSyntax, decodes_new_syntax_add)
{
    ASSERT_NO_THROW(get_instruction_add());
}

TEST_F(InstructionEvaluationFixture, throws_if_syntax_as_no_parameter)
{
    ASSERT_THROW(get_instruction_syntax_without_param(), MissingArgument);
}

TEST_F(InstructionEvaluationFixture, throws_if_context_as_no_parameter)
{
    ASSERT_THROW(get_instruction_context_without_param(), MissingArgument);
}

TEST_F(InstructionEvaluationFixture, throws_if_if_as_no_parameter)
{
    ASSERT_THROW(get_instruction_if_without_param(), MissingArgument);
}

TEST_F(InstructionEvaluationFixture, does_throw_if_macro_does_not_exist)
{
    ASSERT_THROW(get_macro_call(), UndefinedMacro);
}

/// TESTS FOR THE FIRST PASS

TEST_F(FirstPassFixture, does_not_advance_address_for_empty)
{
    auto instruction = get_instruction_empty();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
}

TEST_F(FirstPassFixture, does_not_advance_address_for_end)
{
    auto instruction = get_instruction_end();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
}

TEST_F(FirstPassFixture, does_not_advance_address_for_equ)
{
    auto instruction = get_instruction_equ();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
}

TEST_F(FirstPassFixture, sets_address_for_org)
{
    auto instruction = get_instruction_org();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(0x1000));
}

TEST_F(FirstPassFixture, throws_if_wrong_CPU)
{
    ASSERT_THROW(get_instruction_cpu_unknown(), InvalidCPU);
}

TEST_F(FirstPassFixture, does_not_advance_address_for_correct_cpu)
{
    auto instruction = get_instruction_cpu_known();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
}

TEST_F(FirstPassFixture, advance_address_with_declared_data)
{
    auto instruction = get_instruction_data();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address + 3));
}

TEST_F(FirstPassFixture, throws_if_unknown_opcode)
{
    ASSERT_THROW(get_instruction_invalid_opcode(), UndefinedOpcode);
}

TEST_F(FirstPassFixture, advances_one_byte_if_nop)
{
    auto instruction = get_instruction_nop();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address + 1));
}

TEST_F(FirstPassFixture, syntax_changes_context_and_keeps_address)
{
    auto instruction = get_instruction_syntax();

    context_stack.get_current_context()->get_options().new_syntax = true;

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
    ASSERT_THAT(context_stack.get_current_context()->get_options().new_syntax, IsFalse());
}

TEST_F(FirstPassFixture, context_pushes_context_and_keeps_address)
{
    auto instruction = get_instruction_context();

    const int current_address = 0xff;
    context_stack.get_current_context()->define_symbol("TEST", 123);
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));

    context_pop_and_verify();
}

TEST_F(FirstPassFixture, if_pushes_context_and_sets_parsing_mode_if_true)
{
    auto instruction = get_instruction_if_true();

    const int current_address = 0xff;
    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsTrue());
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsTrue());

    context_pop_and_verify();

    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsTrue());
}

TEST_F(FirstPassFixture, if_pushes_context_and_sets_parsing_mode_if_false)
{
    auto instruction = get_instruction_if_false();

    const int current_address = 0xff;
    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsTrue());
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsFalse());

    context_pop_and_verify();

    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsTrue());
}

TEST_F(FirstPassFixture, else_without_if_throws)
{
    ASSERT_THROW(get_instruction_else(), InvalidConditional);
}

TEST_F(FirstPassFixture, else_uses_if_context_and_sets_parsing_mode_when_if_false)
{
    context_stack.get_current_context()->set_parsing_mode(Context::CONDITIONAL_FALSE);
    auto instruction = get_instruction_else();

    auto initial_context = context_stack.get_current_context();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsTrue());

    // If by changing the current context, we get the same result in the initial context,
    // if means the context was not pushed.
    context_stack.get_current_context()->define_symbol("MARKER", 123);
    const auto [result, value] = initial_context->get_symbol_value("MARKER");
    ASSERT_THAT(result, IsTrue());
}

TEST_F(FirstPassFixture, else_uses_if_context_and_sets_parsing_mode_when_if_true)
{
    context_stack.get_current_context()->set_parsing_mode(Context::CONDITIONAL_TRUE);
    auto instruction = get_instruction_else();

    auto initial_context = context_stack.get_current_context();

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsFalse());

    // If by changing the current context, we get the same result in the initial context,
    // if means the context was not pushed.
    context_stack.get_current_context()->define_symbol("MARKER", 123);
    const auto [result, value] = initial_context->get_symbol_value("MARKER");
    ASSERT_THAT(result, IsTrue());
}

TEST_F(FirstPassFixture, endif_without_if_throws)
{
    ASSERT_THROW(get_instruction_endif(), InvalidConditional);
}

TEST_F(FirstPassFixture, endif_pops_context)
{
    // Simulate a previous "if" conditional.
    context_stack.push();
    context_stack.get_current_context()->set_parsing_mode(Context::CONDITIONAL_TRUE);
    auto instruction = get_instruction_endif();

    // Sets a marker to detect the context stack pop
    context_stack.get_current_context()->define_symbol("MARKER", 123);

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsTrue());

    const auto [result, value] = context_stack.get_current_context()->get_symbol_value("MARKER");
    ASSERT_THAT(result, IsFalse());
}

TEST_F(FirstPassFixture, macro_pushes_context_and_sets_parsing_mode_to_false)
{
    auto instruction = get_instruction_macro();

    const int current_address = 0xff;
    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsTrue());
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsFalse());

    context_pop_and_verify();

    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsTrue());
}

TEST_F(FirstPassFixture, endmacro_without_macro_throws)
{
    ASSERT_THROW(get_instruction_endmacro(), InvalidEndmacro);
}

TEST_F(FirstPassFixture, endmacro_pops_context)
{
    // Simulate a previous "macro" declaration.
    context_stack.push();
    context_stack.get_current_context()->start_macro("a_macro", {});
    auto instruction = get_instruction_endmacro();

    // Sets a marker to detect the context stack pop
    context_stack.get_current_context()->define_symbol("MARKER", 123);

    const int current_address = 0xff;
    ASSERT_THAT(instruction.first_pass(context_stack, current_address), Eq(current_address));
    ASSERT_THAT(context_stack.get_current_context()->is_parsing_active(), IsTrue());

    const auto [result, value] = context_stack.get_current_context()->get_symbol_value("MARKER");
    ASSERT_THAT(result, IsFalse());
}

TEST_F(InstructionEvaluationFixture, macro_call_verifies_number_of_arguments)
{
    // Registers a macro, with one parameter
    context_stack.push();
    context_stack.get_current_context()->start_macro("a_macro", {"param1"});
    context_stack.get_current_context()->stop_macro();
    context_stack.pop();

    auto macro_call_instruction = get_macro_call();

    const int current_address = 0xff;
    ASSERT_THROW(int return_address =
                         macro_call_instruction.first_pass(context_stack, current_address),
                 WrongNumberOfParameters);
}

/// TESTS FOR THE SECOND PASS

TEST_F(SecondPassFixture, does_not_output_byte_if_empty)
{
    auto instruction = get_instruction_empty();

    auto& context = *context_stack.get_current_context();
    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_end)
{
    auto instruction = get_instruction_end();

    auto& context = *context_stack.get_current_context();
    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_equ)
{
    auto instruction = get_instruction_equ();

    auto& context = *context_stack.get_current_context();
    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, does_not_output_byte_if_org)
{
    auto instruction = get_instruction_org();

    auto& context = *context_stack.get_current_context();
    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(0));
}

TEST_F(SecondPassFixture, outputs_declared_data)
{
    auto instruction = get_instruction_data();

    auto& context = *context_stack.get_current_context();
    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(1));
    ASSERT_THAT(byte_buffer.str()[1], Eq(2));
    ASSERT_THAT(byte_buffer.str()[2], Eq(3));
}

TEST_F(SecondPassFixture, outputs_opcode_data)
{
    auto instruction = get_instruction_nop();

    auto& context = *context_stack.get_current_context();
    instruction.second_pass(context, byte_writer, current_address);
    byte_writer.write_end();

    ASSERT_THAT(byte_buffer.str()[0], Eq(static_cast<char>(0xC0)));
    ASSERT_THAT(byte_buffer.str()[1], Eq(0));
}
