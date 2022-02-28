#include "instruction.h"

#include "options.h"
#include "symbol_table.h"

#include "gmock/gmock.h"

using namespace testing;

struct InstructionFixture : public Test
{
    Options options;
    SymbolTable symbol_table;
};

TEST_F(InstructionFixture, returns_the_address_if_empty)
{
    std::string opcode{};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionFixture, returns_the_address_if_end)
{
    std::string opcode{"END"};
    std::vector<std::string> arguments{};

    Instruction instruction{opcode, arguments};

    int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionFixture, returns_the_address_if_cpu)
{
    std::string opcode{"CPU"};
    std::vector<std::string> arguments{"8008"};

    Instruction instruction{opcode, arguments};

    int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address),
                Eq(current_address));
}

TEST_F(InstructionFixture, returns_the_argument_address_if_org)
{
    std::string opcode{"ORG"};
    std::vector<std::string> arguments{"0x1000"};

    Instruction instruction{opcode, arguments};

    int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address), Eq(0x1000));
}

TEST_F(InstructionFixture, returns_the_argument_address_if_equ)
{
    std::string opcode{"EQU"};
    std::vector<std::string> arguments{"0x2000"};

    Instruction instruction{opcode, arguments};

    int current_address = 0xff;
    ASSERT_THAT(instruction.get_evaluation(options, symbol_table, current_address), Eq(0x2000));
}
