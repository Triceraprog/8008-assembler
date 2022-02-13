#include "opcodes.h"
#include "utils.h"
#include <algorithm>
#include <vector>

#define NUMOPCODES (sizeof(opcodes) / sizeof(opcodes[0]))

Opcode opcodes[] = {
        /* first the basic load immediate */
        "lai", 0006, 1, "lbi", 0016, 1, "lci", 0026, 1, "ldi", 0036, 1, "lei", 0046, 1, "lhi", 0056,
        1, "lli", 0066, 1, "lmi", 0076, 1,
        /* now the increment registers */
        "inb", 0010, 0, "inc", 0020, 0, "ind", 0030, 0, "ine", 0040, 0, "inh", 0050, 0, "inl", 0060,
        0,
        /* now decrement registers */
        "dcb", 0011, 0, "dcc", 0021, 0, "dcd", 0031, 0, "dce", 0041, 0, "dch", 0051, 0, "dcl", 0061,
        0,
        /* next add registers to accumulator */
        "ada", 0200, 0, "adb", 0201, 0, "adc", 0202, 0, "add", 0203, 0, "ade", 0204, 0, "adh", 0205,
        0, "adl", 0206, 0, "adm", 0207, 0, "adi", 0004, 1, "aca", 0210, 0, "acb", 0211, 0, "acc",
        0212, 0, "acd", 0213, 0, "ace", 0214, 0, "ach", 0215, 0, "acl", 0216, 0, "acm", 0217, 0,
        "aci", 0014, 1,
        /* next subtract registers from accumulator */
        "sua", 0220, 0, "sub", 0221, 0, "suc", 0222, 0, "sud", 0223, 0, "sue", 0224, 0, "suh", 0225,
        0, "sul", 0226, 0, "sum", 0227, 0, "sui", 0024, 1, "sba", 0230, 0, "sbb", 0231, 0, "sbc",
        0232, 0, "sbd", 0233, 0, "sbe", 0234, 0, "sbh", 0235, 0, "sbl", 0236, 0, "sbm", 0237, 0,
        "sbi", 0034, 1,
        /* and registers with accumulator */
        "nda", 0240, 0, "ndb", 0241, 0, "ndc", 0242, 0, "ndd", 0243, 0, "nde", 0244, 0, "ndh", 0245,
        0, "ndl", 0246, 0, "ndm", 0247, 0, "ndi", 0044, 1,
        /* xor registers with accumulator */
        "xra", 0250, 0, "xrb", 0251, 0, "xrc", 0252, 0, "xrd", 0253, 0, "xre", 0254, 0, "xrh", 0255,
        0, "xrl", 0256, 0, "xrm", 0257, 0, "xri", 0054, 1,
        /* or registers with accumulator */
        "ora", 0260, 0, "orb", 0261, 0, "orc", 0262, 0, "ord", 0263, 0, "ore", 0264, 0, "orh", 0265,
        0, "orl", 0266, 0, "orm", 0267, 0, "ori", 0064, 1,
        /* compare registers with accumulator */
        "cpa", 0270, 0, "cpb", 0271, 0, "cpc", 0272, 0, "cpd", 0273, 0, "cpe", 0274, 0, "cph", 0275,
        0, "cpl", 0276, 0, "cpm", 0277, 0, "cpi", 0074, 1,
        /* a halt code */
        "hlt", 0001, 0,
        /* now all the load registers */
        "laa", 0300, 0, "lab", 0301, 0, "lac", 0302, 0, "lad", 0303, 0, "lae", 0304, 0, "lah", 0305,
        0, "lal", 0306, 0, "lam", 0307, 0, "lba", 0310, 0, "lbb", 0311, 0, "lbc", 0312, 0, "lbd",
        0313, 0, "lbe", 0314, 0, "lbh", 0315, 0, "lbl", 0316, 0, "lbm", 0317, 0, "lca", 0320, 0,
        "lcb", 0321, 0, "lcc", 0322, 0, "lcd", 0323, 0, "lce", 0324, 0, "lch", 0325, 0, "lcl", 0326,
        0, "lcm", 0327, 0, "lda", 0330, 0, "ldb", 0331, 0, "ldc", 0332, 0, "ldd", 0333, 0, "lde",
        0334, 0, "ldh", 0335, 0, "ldl", 0336, 0, "ldm", 0337, 0, "lea", 0340, 0, "leb", 0341, 0,
        "lec", 0342, 0, "led", 0343, 0, "lee", 0344, 0, "leh", 0345, 0, "lel", 0346, 0, "lem", 0347,
        0, "lha", 0350, 0, "lhb", 0351, 0, "lhc", 0352, 0, "lhd", 0353, 0, "lhe", 0354, 0, "lhh",
        0355, 0, "lhl", 0356, 0, "lhm", 0357, 0, "lla", 0360, 0, "llb", 0361, 0, "llc", 0362, 0,
        "lld", 0363, 0, "lle", 0364, 0, "llh", 0365, 0, "lll", 0366, 0, "llm", 0367, 0, "lma", 0370,
        0, "lmb", 0371, 0, "lmc", 0372, 0, "lmd", 0373, 0, "lme", 0374, 0, "lmh", 0375, 0, "lml",
        0376, 0,
        /* rotate the accumulator */
        "ral", 0022, 0, "rar", 0032, 0, "rlc", 0002, 0, "rrc", 0012, 0,
        /* jump instructions */
        "jmp", 0104, 2, "jfc", 0100, 2, "jfz", 0110, 2, "jfs", 0120, 2, "jfp", 0130, 2, "jtc", 0140,
        2, "jtz", 0150, 2, "jts", 0160, 2, "jtp", 0170, 2,
        /* call instructions */
        "cal", 0106, 2, "cfc", 0102, 2, "cfz", 0112, 2, "cfs", 0122, 2, "cfp", 0132, 2, "ctc", 0142,
        2, "ctz", 0152, 2, "cts", 0162, 2, "ctp", 0172, 2, "rst", 0005, 4,
        /* return instructions */
        "ret", 0007, 0, "rfc", 0003, 0, "rfz", 0013, 0, "rfs", 0023, 0, "rfp", 0033, 0, "rtc", 0043,
        0, "rtz", 0053, 0, "rts", 0063, 0, "rtp", 0073, 0,
        /* input and output */
        "inp", 0101, 3, "out", 0121, 3};

/* The above rules in the table are:
 *  0:no argument needed,
 *  1:immediate byte follows
 *  2:two byte address follows
 *  3:output port number follows
 *  4:rst syntax
 */

namespace
{
    int find_opcode_in_table(std::string_view str)
    {
        for (int i = 0; i < NUMOPCODES; i++)
        {
            if (ci_equals(str, opcodes[i].mnemonic))
            {
                return i;
            }
        }
        return -1;
    }

    Opcode& get_opcode(int i) { return opcodes[i]; }

}

std::tuple<bool, Opcode&> find_opcode(std::string_view opcode_name)
{
    static Opcode null_opcode;

    int index = find_opcode_in_table(opcode_name);
    if (index == -1)
    {
        return {false, null_opcode};
    }

    return {true, get_opcode(index)};
}

PseudoOpcodeEnum opcode_to_enum(std::string_view opcode)
{
    static std::vector<std::tuple<const char*, PseudoOpcodeEnum>> association = {
            {"equ", PseudoOpcodeEnum::EQU},   {"end", PseudoOpcodeEnum::END},
            {"cpu", PseudoOpcodeEnum::CPU},   {"org", PseudoOpcodeEnum::ORG},
            {"data", PseudoOpcodeEnum::DATA},
    };
    if (opcode.empty())
    {
        return PseudoOpcodeEnum::EMPTY;
    }
    auto found_op_code = std::ranges::find_if(association, [&opcode](const auto& t) {
        auto& [opcode_str, opcode_enum] = t;
        return ci_equals(opcode, opcode_str);
    });

    if (found_op_code == association.end())
    {
        return PseudoOpcodeEnum::OTHER;
    }

    return std::get<1>(*found_op_code);
}

int get_opcode_size(const Opcode& opcode)
{
    int opcode_byte_size;
    switch (opcode.rule)
    {
        case 0:
        case 3:
        case 4:
            opcode_byte_size = 1;
            break;
        case 1:
            opcode_byte_size = 2;
            break;
        case 2:
            opcode_byte_size = 3;
            break;
    }
    return opcode_byte_size;
}

UndefinedOpcode::UndefinedOpcode(const std::string& opcode)
{
    reason = "undefined opcode " + opcode;
}
