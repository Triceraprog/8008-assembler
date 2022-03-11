#include "opcodes/opcodes.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(Opcode, can_select_the_opcode_matcher)
{
    auto matcher = get_opcode_matcher(OLD);

    std::vector<std::string> arguments{};
    auto [found, found_opcode, consume] = matcher("LAB", arguments);

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(NO_ARG));
    ASSERT_THAT(found_opcode.code, Eq(0301));
}

struct OldOpcodeSyntaxFixture : public Test
{
    std::vector<std::string> arguments{};
};

TEST_F(OldOpcodeSyntaxFixture, understands_no_arg_opcode)
{
    auto [found, found_opcode, consume] = find_opcode("LAA", arguments);

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(NO_ARG));
    ASSERT_THAT(found_opcode.code, Eq(0300));
}

TEST_F(OldOpcodeSyntaxFixture, understands_one_byte_arg_opcode)
{
    std::vector<std::string> arguments{};
    auto [found, found_opcode, consume] = find_opcode("LAI", arguments);

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(ONE_BYTE_ARG));
    ASSERT_THAT(found_opcode.code, Eq(0006));
}

TEST_F(OldOpcodeSyntaxFixture, understands_address_arg_opcode)
{
    std::vector<std::string> arguments{};
    auto [found, found_opcode, consume] = find_opcode("JMP", arguments);

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(ADDRESS_ARG));
    ASSERT_THAT(found_opcode.code, Eq(0104));
}

TEST_F(OldOpcodeSyntaxFixture, understands_inpout_opcode)
{
    std::vector<std::string> arguments{};
    auto [found, found_opcode, consume] = find_opcode("INP", arguments);

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(INP_OUT));
    ASSERT_THAT(found_opcode.code, Eq(0101));
}

TEST_F(OldOpcodeSyntaxFixture, understands_rst_opcode)
{
    std::vector<std::string> arguments{};
    auto [found, found_opcode, consume] = find_opcode("RST", arguments);

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(RST));
    ASSERT_THAT(found_opcode.code, Eq(0005));
}

struct NewOpcodeSyntaxFixture : public Test
{
};

TEST_F(NewOpcodeSyntaxFixture, understands_mov_opcode)
{
    std::vector<std::string> arguments{"A", "B"};
    auto [found, found_opcode, consumed] = find_opcode("MOV", arguments);

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(NO_ARG));
    ASSERT_THAT(found_opcode.code, Eq(0301));
    ASSERT_THAT(consumed, Eq(2));
}

TEST_F(NewOpcodeSyntaxFixture, mov_missing_one_argument_is_a_syntax_error)
{
    std::vector<std::string> arguments{"A"};
    ASSERT_THROW(find_opcode("MOV", arguments), SyntaxError);
}

TEST_F(NewOpcodeSyntaxFixture, mov_missing_first_argument_is_a_syntax_error)
{
    std::vector<std::string> arguments{"", "B"};
    ASSERT_THROW(find_opcode("MOV", arguments), SyntaxError);
}

TEST_F(NewOpcodeSyntaxFixture, understands_mov_opcode_with_different_registers)
{
    std::vector<std::string> arguments{"M", "E"};
    auto [found, found_opcode, consumed] = find_opcode("MOV", arguments);

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(NO_ARG));
    ASSERT_THAT(found_opcode.code, Eq(0374));
    ASSERT_THAT(consumed, Eq(2));
}

TEST_F(NewOpcodeSyntaxFixture, understands_mvi_opcode)
{
    std::vector<std::string> arguments{"C"};
    auto [found, found_opcode, consumed] = find_opcode("MVI", arguments);

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(ONE_BYTE_ARG));
    ASSERT_THAT(found_opcode.code, Eq(0026));
    ASSERT_THAT(consumed, Eq(1));
}

TEST_F(NewOpcodeSyntaxFixture, mov_with_a_number_as_first_argument_is_a_syntax_error)
{
    std::vector<std::string> arguments{"1", "B"};
    ASSERT_THROW(find_opcode("MVI", arguments), SyntaxError);
}

/*
TEST_F(OldOpcodeSyntaxFixture, understands_add_opcode)
{
    std::vector<std::string> arguments{"D"};
    auto [found, found_opcode, consumed] = find_opcode("ADD", arguments);

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(ONE_BYTE_ARG));
    ASSERT_THAT(found_opcode.code, Eq(0203));
    ASSERT_THAT(consumed, Eq(1));
}
*/