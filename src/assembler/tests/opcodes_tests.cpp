#include "opcodes.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(PseudoOpcodes, can_be_parsed_as_enum)
{
    ASSERT_THAT(opcode_to_enum(""), Eq(PseudoOpcodeEnum::EMPTY));

    ASSERT_THAT(opcode_to_enum("EQU"), Eq(PseudoOpcodeEnum::EQU));
    ASSERT_THAT(opcode_to_enum("equ"), Eq(PseudoOpcodeEnum::EQU));

    ASSERT_THAT(opcode_to_enum("END"), Eq(PseudoOpcodeEnum::END));
    ASSERT_THAT(opcode_to_enum("end"), Eq(PseudoOpcodeEnum::END));

    ASSERT_THAT(opcode_to_enum("CPU"), Eq(PseudoOpcodeEnum::CPU));
    ASSERT_THAT(opcode_to_enum("cpu"), Eq(PseudoOpcodeEnum::CPU));

    ASSERT_THAT(opcode_to_enum("ORG"), Eq(PseudoOpcodeEnum::ORG));
    ASSERT_THAT(opcode_to_enum("org"), Eq(PseudoOpcodeEnum::ORG));

    ASSERT_THAT(opcode_to_enum("DATA"), Eq(PseudoOpcodeEnum::DATA));
    ASSERT_THAT(opcode_to_enum("data"), Eq(PseudoOpcodeEnum::DATA));

    ASSERT_THAT(opcode_to_enum("LAA"), Eq(PseudoOpcodeEnum::OTHER));
    ASSERT_THAT(opcode_to_enum("garbage"), Eq(PseudoOpcodeEnum::OTHER));
}
