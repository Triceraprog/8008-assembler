#include "opcodes/opcodes.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(Opcode, understands_no_arg_opcode)
{
    auto [found, found_opcode] = find_opcode("LAA");

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(NO_ARG));
    ASSERT_THAT(found_opcode.code, Eq(0300));
}

TEST(Opcode, understands_one_byte_arg_opcode)
{
    auto [found, found_opcode] = find_opcode("LAI");

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(ONE_BYTE_ARG));
    ASSERT_THAT(found_opcode.code, Eq(0006));
}

TEST(Opcode, understands_address_arg_opcode)
{
    auto [found, found_opcode] = find_opcode("JMP");

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(ADDRESS_ARG));
    ASSERT_THAT(found_opcode.code, Eq(0104));
}

TEST(Opcode, understands_inpout_opcode)
{
    auto [found, found_opcode] = find_opcode("INP");

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(INP_OUT));
    ASSERT_THAT(found_opcode.code, Eq(0101));
}

TEST(Opcode, understands_rst_opcode)
{
    auto [found, found_opcode] = find_opcode("RST");

    ASSERT_THAT(found, IsTrue());
    ASSERT_THAT(found_opcode.rule, Eq(RST));
    ASSERT_THAT(found_opcode.code, Eq(0005));
}
