#include "opcodes.h"

#include "utils.h"

#include <algorithm>
#include <cassert>

namespace
{
    Opcode opcodes[] = {
            /* first the basic load immediate */
            "lai", 0006, ONE_BYTE_ARG, "lbi", 0016, ONE_BYTE_ARG, //
            "lci", 0026, ONE_BYTE_ARG, "ldi", 0036, ONE_BYTE_ARG, //
            "lei", 0046, ONE_BYTE_ARG, "lhi", 0056, ONE_BYTE_ARG, //
            "lli", 0066, ONE_BYTE_ARG, "lmi", 0076, ONE_BYTE_ARG, //
            /* now the increment registers */
            "inb", 0010, NO_ARG, "inc", 0020, NO_ARG, "ind", 0030, NO_ARG, "ine", 0040, NO_ARG, //
            "inh", 0050, NO_ARG, "inl", 0060, NO_ARG,
            /* now decrement registers */
            "dcb", 0011, NO_ARG, "dcc", 0021, NO_ARG, "dcd", 0031, NO_ARG, "dce", 0041, NO_ARG, //
            "dch", 0051, NO_ARG, "dcl", 0061, NO_ARG,
            /* next add registers to accumulator */
            "ada", 0200, NO_ARG, "adb", 0201, NO_ARG, "adc", 0202, NO_ARG, "add", 0203, NO_ARG, //
            "ade", 0204, NO_ARG, "adh", 0205, NO_ARG, "adl", 0206, NO_ARG, "adm", 0207, NO_ARG, //
            "adi", 0004, ONE_BYTE_ARG,                                                          //
            "aca", 0210, NO_ARG, "acb", 0211, NO_ARG, "acc", 0212, NO_ARG, "acd", 0213, NO_ARG, //
            "ace", 0214, NO_ARG, "ach", 0215, NO_ARG, "acl", 0216, NO_ARG, "acm", 0217, NO_ARG, //
            "aci", 0014, ONE_BYTE_ARG,                                                          //
            /* next subtract registers from accumulator */
            "sua", 0220, NO_ARG, "sub", 0221, NO_ARG, "suc", 0222, NO_ARG, "sud", 0223, NO_ARG, //
            "sue", 0224, NO_ARG, "suh", 0225, NO_ARG, "sul", 0226, NO_ARG, "sum", 0227, NO_ARG, //
            "sui", 0024, ONE_BYTE_ARG,                                                          //
            "sba", 0230, NO_ARG, "sbb", 0231, NO_ARG, "sbc", 0232, NO_ARG, "sbd", 0233, NO_ARG, //
            "sbe", 0234, NO_ARG, "sbh", 0235, NO_ARG, "sbl", 0236, NO_ARG, "sbm", 0237, NO_ARG, //
            "sbi", 0034, ONE_BYTE_ARG,
            /* and registers with accumulator */
            "nda", 0240, NO_ARG, "ndb", 0241, NO_ARG, "ndc", 0242, NO_ARG, "ndd", 0243, NO_ARG, //
            "nde", 0244, NO_ARG, "ndh", 0245, NO_ARG, "ndl", 0246, NO_ARG, "ndm", 0247, NO_ARG, //
            "ndi", 0044, ONE_BYTE_ARG,
            /* xor registers with accumulator */
            "xra", 0250, NO_ARG, "xrb", 0251, NO_ARG, "xrc", 0252, NO_ARG, "xrd", 0253, NO_ARG, //
            "xre", 0254, NO_ARG, "xrh", 0255, NO_ARG, "xrl", 0256, NO_ARG, "xrm", 0257, NO_ARG, //
            "xri", 0054, ONE_BYTE_ARG,
            /* or registers with accumulator */
            "ora", 0260, NO_ARG, "orb", 0261, NO_ARG, "orc", 0262, NO_ARG, "ord", 0263, NO_ARG, //
            "ore", 0264, NO_ARG, "orh", 0265, NO_ARG, "orl", 0266, NO_ARG, "orm", 0267, NO_ARG, //
            "ori", 0064, ONE_BYTE_ARG,
            /* compare registers with accumulator */
            "cpa", 0270, NO_ARG, "cpb", 0271, NO_ARG, "cpc", 0272, NO_ARG, "cpd", 0273, NO_ARG, //
            "cpe", 0274, NO_ARG, "cph", 0275, NO_ARG, "cpl", 0276, NO_ARG, "cpm", 0277, NO_ARG, //
            "cpi", 0074, ONE_BYTE_ARG,
            /* a halt code */
            "hlt", 0001, NO_ARG, //
            /* now all the load registers */
            "laa", 0300, NO_ARG, "lab", 0301, NO_ARG, "lac", 0302, NO_ARG, "lad", 0303, NO_ARG, //
            "lae", 0304, NO_ARG, "lah", 0305, NO_ARG, "lal", 0306, NO_ARG, "lam", 0307, NO_ARG, //
            "lba", 0310, NO_ARG, "lbb", 0311, NO_ARG, "lbc", 0312, NO_ARG, "lbd", 0313, NO_ARG, //
            "lbe", 0314, NO_ARG, "lbh", 0315, NO_ARG, "lbl", 0316, NO_ARG, "lbm", 0317, NO_ARG, //
            "lca", 0320, NO_ARG, "lcb", 0321, NO_ARG, "lcc", 0322, NO_ARG, "lcd", 0323, NO_ARG, //
            "lce", 0324, NO_ARG, "lch", 0325, NO_ARG, "lcl", 0326, NO_ARG, "lcm", 0327, NO_ARG, //
            "lda", 0330, NO_ARG, "ldb", 0331, NO_ARG, "ldc", 0332, NO_ARG, "ldd", 0333, NO_ARG, //
            "lde", 0334, NO_ARG, "ldh", 0335, NO_ARG, "ldl", 0336, NO_ARG, "ldm", 0337, NO_ARG, //
            "lea", 0340, NO_ARG, "leb", 0341, NO_ARG, "lec", 0342, NO_ARG, "led", 0343, NO_ARG, //
            "lee", 0344, NO_ARG, "leh", 0345, NO_ARG, "lel", 0346, NO_ARG, "lem", 0347, NO_ARG, //
            "lha", 0350, NO_ARG, "lhb", 0351, NO_ARG, "lhc", 0352, NO_ARG, "lhd", 0353, NO_ARG, //
            "lhe", 0354, NO_ARG, "lhh", 0355, NO_ARG, "lhl", 0356, NO_ARG, "lhm", 0357, NO_ARG, //
            "lla", 0360, NO_ARG, "llb", 0361, NO_ARG, "llc", 0362, NO_ARG, "lld", 0363, NO_ARG, //
            "lle", 0364, NO_ARG, "llh", 0365, NO_ARG, "lll", 0366, NO_ARG, "llm", 0367, NO_ARG, //
            "lma", 0370, NO_ARG, "lmb", 0371, NO_ARG, "lmc", 0372, NO_ARG, "lmd", 0373, NO_ARG, //
            "lme", 0374, NO_ARG, "lmh", 0375, NO_ARG, "lml", 0376, NO_ARG,
            /* rotate the accumulator */
            "ral", 0022, NO_ARG, "rar", 0032, NO_ARG, "rlc", 0002, NO_ARG, "rrc", 0012, NO_ARG,
            /* jump instructions */
            "jmp", 0104, ADDRESS_ARG, "jfc", 0100, ADDRESS_ARG, "jfz", 0110, ADDRESS_ARG, //
            "jfs", 0120, ADDRESS_ARG, "jfp", 0130, ADDRESS_ARG, "jtc", 0140, ADDRESS_ARG, //
            "jtz", 0150, ADDRESS_ARG, "jts", 0160, ADDRESS_ARG, "jtp", 0170, ADDRESS_ARG,
            // new syntax
            "jnc", 0100, ADDRESS_ARG, "jnz", 0110, ADDRESS_ARG,                         //
            "jp", 0120, ADDRESS_ARG, "jpo", 0130, ADDRESS_ARG, "jc", 0140, ADDRESS_ARG, //
            "jz", 0150, ADDRESS_ARG, "jm", 0160, ADDRESS_ARG, "jpe", 0170, ADDRESS_ARG,
            /* call instructions */
            "cal", 0106, ADDRESS_ARG, "cfc", 0102, ADDRESS_ARG, "cfz", 0112, ADDRESS_ARG, //
            "cfs", 0122, ADDRESS_ARG, "cfp", 0132, ADDRESS_ARG, "ctc", 0142, ADDRESS_ARG, //
            "ctz", 0152, ADDRESS_ARG, "cts", 0162, ADDRESS_ARG, "ctp", 0172, ADDRESS_ARG, //
            "rst", 0005, RST,
            // new syntax
            "call", 0106, ADDRESS_ARG, "cnc", 0102, ADDRESS_ARG, "cnz", 0112, ADDRESS_ARG, //
            "cp", 0122, ADDRESS_ARG, "cpo", 0132, ADDRESS_ARG, "cc", 0142, ADDRESS_ARG,    //
            "cz", 0152, ADDRESS_ARG, "cm", 0162, ADDRESS_ARG, "cpe", 0172, ADDRESS_ARG,    //
            /* return instructions */
            "ret", 0007, NO_ARG, "rfc", 0003, NO_ARG, "rfz", 0013, NO_ARG, "rfs", 0023, NO_ARG, //
            "rfp", 0033, NO_ARG, "rtc", 0043, NO_ARG, "rtz", 0053, NO_ARG, "rts", 0063, NO_ARG, //
            "rtp", 0073, NO_ARG,
            /* input and output */
            "inp", 0101, INP_OUT, "out", 0121, INP_OUT, "in", 0101, INP_OUT, // new syntax
            /* micral specific aliases to instructions */
            "mas", 0322, NO_ARG, "dms", 0366, NO_ARG, "rei", 0037, NO_ARG};

    enum NewSyntaxSourceDest
    {
        SOURCE,
        DESTINATION,
        SOURCE_AND_DESTINATION,
    };

    struct NewSyntaxOpcode
    {
        using OpcodeByteType = unsigned char;
        const char* mnemonic{};
        OpcodeByteType code{};
        OpcodeType rule{};
        NewSyntaxSourceDest source_and_dest{};
    };

    NewSyntaxOpcode new_opcodes[] = {
            "mov", 0b11000000, NO_ARG,       SOURCE_AND_DESTINATION, //
            "mvi", 0b00000110, ONE_BYTE_ARG, DESTINATION             //
    };

    std::tuple<bool, Opcode> find_old_opcode(std::string_view opcode_name)
    {
        static Opcode null_opcode;

        auto it = std::ranges::find_if(opcodes, [&opcode_name](const auto& opcode) {
            return ci_equals(opcode_name, opcode.mnemonic);
        });

        if (it == std::end(opcodes))
        {
            return {false, null_opcode};
        }
        return {true, *it};
    }

    std::tuple<bool, NewSyntaxOpcode> find_new_opcode(std::string_view opcode_name)
    {
        static NewSyntaxOpcode null_opcode;

        auto it = std::ranges::find_if(new_opcodes, [&opcode_name](const auto& opcode) {
            return ci_equals(opcode_name, opcode.mnemonic);
        });
        if (it == std::end(new_opcodes))
        {
            return {false, null_opcode};
        }

        return {true, *it};
    }

    void verify_arguments_count(const std::string_view instruction_name,
                                const std::span<std::string>& arguments, const int argument_needed)
    {
        if (arguments.size() < argument_needed)
        {
            throw SyntaxError("missing argument(s) for " + std::string{instruction_name});
        }

        const auto& first_argument = arguments[0];
        if (argument_needed >= 1 && first_argument.size() != 1)
        {
            throw SyntaxError("Wrong first argument for " + std::string{instruction_name});
        }

        const auto& second_argument = arguments[1];
        if (argument_needed >= 2 && second_argument.size() != 1)
        {
            throw SyntaxError("Wrong second argument for " + std::string{instruction_name});
        }
    }

    int reg_name_to_code(std::string_view register_name)
    {
        assert(register_name.size() == 1);
        const auto& reg_char = register_name[0];
        if (reg_char == 'M')
        {
            return 7;
        }
        if (reg_char >= 'A' && reg_char <= 'E')
        {
            return reg_char - 'A';
        }
        throw SyntaxError("Register expected.");
    }
}

std::tuple<bool, Opcode, std::size_t> find_opcode(std::string_view opcode_name,
                                                  std::span<std::string> arguments)
{
    const auto& [found, opcode] = find_old_opcode(opcode_name);

    if (!found)
    {
        const auto& [new_found, new_opcode] = find_new_opcode(opcode_name);
        if (new_found)
        {
            const int argument_needed =
                    (new_opcode.source_and_dest == SOURCE_AND_DESTINATION) ? 2 : 1;
            verify_arguments_count(opcode_name, arguments, argument_needed);

            Opcode::OpcodeByteType code = new_opcode.code;
            if (new_opcode.source_and_dest == SOURCE_AND_DESTINATION)
            {
                const int destination_reg = reg_name_to_code(arguments[0]);
                const int source_reg = reg_name_to_code(arguments[1]);
                code |= (destination_reg) << 3 | source_reg;
            }
            else if (new_opcode.source_and_dest == SOURCE)
            {
                const int source_reg = reg_name_to_code(arguments[0]);
                code |= source_reg;
            }
            else
            {
                const int destination_reg = reg_name_to_code(arguments[0]);
                code |= (destination_reg) << 3;
            }

            Opcode new_syntax_opcode{new_opcode.mnemonic, code, new_opcode.rule};

            return {true, new_syntax_opcode, argument_needed};
        }
    }
    return {found, opcode, 0};
}

int get_opcode_size(const Opcode& opcode)
{
    int opcode_byte_size;
    switch (opcode.rule)
    {
        case NO_ARG:
        case INP_OUT:
        case RST:
            opcode_byte_size = 1;
            break;
        case ONE_BYTE_ARG:
            opcode_byte_size = 2;
            break;
        case ADDRESS_ARG:
            opcode_byte_size = 3;
            break;
    }
    return opcode_byte_size;
}

UndefinedOpcode::UndefinedOpcode(std::string_view opcode)
{
    reason = "undefined opcode " + std::string{opcode};
}

SyntaxError::SyntaxError(std::string_view reason) { this->reason = reason; }
