#include "opcode_action.h"

#include "byte_writer.h"
#include "evaluator.h"
#include "listing.h"

#include "opcode_action_inpout.h"
#include "opcode_action_noarg.h"
#include "opcode_action_onebyte_arg.h"
#include "opcode_action_rst.h"
#include "opcode_action_twobyte_arg.h"

namespace
{
    bool correct_argument_count(const Opcode& opcode, uint32_t arg_count)
    {
        return ((opcode.rule == 0) && (arg_count != 0)) ||
               ((opcode.rule == 1) && (arg_count != 1)) ||
               ((opcode.rule == 2) && (arg_count != 1)) ||
               ((opcode.rule == 3) && (arg_count != 1)) || ((opcode.rule == 4) && (arg_count != 1));
    }
}

std::unique_ptr<OpcodeAction> create_opcode_action(const Context& context, Opcode opcode,
                                                   int address,
                                                   const std::vector<std::string>& arguments)
{
    if (correct_argument_count(opcode, arguments.size()))
    {
        throw UnexpectedArgumentCount(arguments.size());
    }

    switch (opcode.rule)
    {
        case NO_ARG:
            return std::make_unique<OpcodeActionNoArg>(opcode.code, address);
        case ONE_BYTE_ARG:
            return std::make_unique<OpcodeActionOneByteArg>(context, opcode.code, address,
                                                            arguments);
        case ADDRESS_ARG:
            return std::make_unique<OpcodeActionTwoByteArg>(context, opcode.code, address,
                                                            arguments);
        case INP_OUT:
            return std::make_unique<OpcodeActionInpOut>(context, opcode.code, address, arguments,
                                                        opcode.mnemonic);
        case RST:
            return std::make_unique<OpcodeActionRst>(context, opcode.code, address, arguments);
    }
    return nullptr;
}

ExpectedArgumentWithinLimits::ExpectedArgumentWithinLimits(int limit, const std::string& content,
                                                           int evaluated)
{
    reason = "expected argument between 0 and " + std::to_string(limit) + " instead got " +
             content + "=" + std::to_string(evaluated);
}

UnexpectedArgumentCount::UnexpectedArgumentCount(uint32_t arg_count)
{
    reason = "unexpected number of arguments: " + std::to_string(arg_count);
}

int OpcodeAction::evaluate(const Context& context, std::string_view arg)
{
    return evaluate_argument(context, arg);
}
