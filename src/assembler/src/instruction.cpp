#include "instruction.h"
#include "byte_writer.h"
#include "context.h"
#include "context_stack.h"
#include "data_extraction.h"
#include "evaluator.h"
#include "files/file_utility.h"
#include "files/files.h"
#include "listing.h"
#include "opcodes/opcode_action.h"
#include "opcodes/opcodes.h"
#include "options.h"
#include "utils.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

namespace
{
    struct Validated_Instruction : public Instruction::InstructionAction
    {
        Validated_Instruction(std::string_view name, const std::vector<std::string>& arguments)
        {
            if (arguments.empty())
            {
                throw MissingArgument(name);
            }
        }
    };

    struct Instruction_EQU : public Validated_Instruction
    {
        explicit Instruction_EQU(const std::vector<std::string>& arguments)
            : Validated_Instruction("EQU", arguments)
        {
            first_arg = arguments[0];
        }

        [[nodiscard]] int evaluate_fixed_address(const Context& context,
                                                 int current_address) const override
        {
            return evaluate_argument(context, first_arg);
        }

        std::string first_arg;
    };

    struct Instruction_END : public Instruction::InstructionAction
    {
        // For END, we could stock the evaluation, but rather than that
        // we will go ahead and check for more.
        // Said otherwise: END is ignored.
    };

    struct Instruction_CPU : public Validated_Instruction
    {
        explicit Instruction_CPU(const std::vector<std::string>& arguments)
            : Validated_Instruction("CPU", arguments)
        {
            verify_cpu(arguments[0]);
        }

        static void verify_cpu(const std::string& cpu_arg)
        {
            if ((!ci_equals(cpu_arg, "8008")) && (!ci_equals(cpu_arg, "i8008")))
            {
                throw InvalidCPU();
            }
        }
    };

    struct Instruction_ORG : public Validated_Instruction
    {
        Instruction_ORG(const Context& context, const std::vector<std::string>& arguments)
            : Validated_Instruction("ORG", arguments)
        {
            evaluated_argument = evaluate_argument(context, arguments[0]);
        }

        [[nodiscard]] int evaluate_fixed_address(const Context& context,
                                                 int current_address) const override
        {
            return evaluated_argument;
        }

        [[nodiscard]] int advance_address(const Context& context,
                                          int current_address) const override
        {
            return evaluated_argument;
        }

        int evaluated_argument;
    };

    struct Instruction_DATA : public Instruction::InstructionAction
    {
        Instruction_DATA(const Context& context, const std::vector<std::string>& arguments)
        {
            data_size = decode_data(context, arguments, data_list);

            if (context.get_options().debug)
            {
                std::cout << "got " << std::dec << std::abs(data_size) << " items in data list\n";
            }
        }

        [[nodiscard]] int advance_address(const Context& context,
                                          int current_address) const override
        {
            /* a negative number denotes that much space to save, but not specifying data */
            return current_address + std::abs(data_size);
        }

        void write_bytes(const Context& context, ByteWriter& writer, int address) const override
        {
            for (int write_address = address; const auto& data : data_list)
            {
                writer.write_byte(data, write_address);
                write_address += 1;
            }
        }

        void write_listing(Listing& listing, const std::string& input_line, uint32_t line_number,
                           int address) const override
        {
            if (data_size < 0)
            {
                /* if n is negative, that number of bytes are just reserved */
                listing.reserved_data(line_number, address, input_line);
            }
            else
            {
                listing.data(line_number, address, input_line, data_list);
            }
        }

        std::vector<int> data_list;
        int data_size; // Can be negative in case of uninitialized data reservation.
    };

    struct Instruction_OTHER : public Instruction::InstructionAction
    {
        Instruction_OTHER(const Context& context, std::string_view opcode_string,
                          std::vector<std::string> arguments, SyntaxType syntax_type)
        {
            if (context.get_options().debug)
            {
                std::cout << "\n";
            }

            auto find_opcode = get_opcode_matcher(syntax_type);
            if (auto [found, found_opcode, consumed] = find_opcode(opcode_string, arguments); found)
            {
                opcode = found_opcode;
                if (consumed == 0)
                {
                    this->arguments = std::move(arguments);
                }
                else
                {
                    assert(consumed <= arguments.size());
                    std::ranges::copy(arguments | std::views::drop(consumed),
                                      std::back_inserter(this->arguments));
                }
            }
            else
            {
                throw UndefinedOpcode(opcode_string);
            }
        }

        [[nodiscard]] int advance_address(const Context& context,
                                          int current_address) const override
        {
            return current_address + get_opcode_size(opcode);
        }

        void build(const Context& context, int address) override
        {
            opcode_action = std::move(create_opcode_action(context, opcode, address, arguments));
        }

        void write_bytes(const Context& context, ByteWriter& writer, int address) const override
        {
            opcode_action->emit_byte_stream(writer);
        }

        void write_listing(Listing& listing, const std::string& input_line, uint32_t line_number,
                           int address) const override
        {
            opcode_action->emit_listing(listing, line_number, input_line);
        }

        std::vector<std::string> arguments;
        std::unique_ptr<OpcodeAction> opcode_action;
        Opcode opcode;
    };

    struct Instruction_INCLUDE : public Validated_Instruction
    {
        Instruction_INCLUDE(const Context& context, const std::vector<std::string>& arguments,
                            FileReader& file_reader)
            : Validated_Instruction(".include", arguments)
        {
            auto include_filename = arguments[0];

            if (context.get_options().debug)
            {
                std::cout << "got '" << include_filename << "' as a filename to include.\n";
            }

            Utility::insert_file_by_name(file_reader, include_filename);
        }
    };

    struct Instruction_SYNTAX : public Validated_Instruction
    {
        Instruction_SYNTAX(const Context& context, const std::vector<std::string>& arguments)
            : Validated_Instruction(".syntax", arguments)
        {
            auto syntax_type = arguments[0];

            if (context.get_options().debug)
            {
                std::cout << "got '" << syntax_type << "' as the new syntax.\n";
            }

            verify_syntax(syntax_type);
            new_syntax = ci_equals(syntax_type, "NEW");
        }

        static void verify_syntax(const std::string& syntax)
        {
            if ((!ci_equals(syntax, "OLD")) && (!ci_equals(syntax, "NEW")))
            {
                throw InvalidSyntax();
            }
        }

        void update_context_stack(ContextStack& context_stack) const override
        {
            // The syntax instruction changes the current syntax mode.
            context_stack.get_current_context()->get_options().new_syntax = new_syntax;
            InstructionAction::update_context_stack(context_stack);
        }

        bool new_syntax{};
    };

    struct Instruction_CONTEXT : public Validated_Instruction
    {
        Instruction_CONTEXT(const Context& context, const std::vector<std::string>& arguments)
            : Validated_Instruction(".context", arguments)
        {
            auto context_action = arguments[0];

            if (context.get_options().debug)
            {
                std::cout << "got '" << context_action << "' as the context action.\n";
            }

            verify_syntax(context_action);
            action = ci_equals(context_action, "POP") ? POP : PUSH;
        }

        static void verify_syntax(const std::string& syntax)
        {
            if ((!ci_equals(syntax, "PUSH")) && (!ci_equals(syntax, "POP")))
            {
                throw InvalidContextAction();
            }
        }

        void update_context_stack(ContextStack& context_stack) const override
        {
            switch (action)
            {
                case PUSH:
                    context_stack.push();
                    break;
                case POP:
                    context_stack.pop();
                    break;
            }
            InstructionAction::update_context_stack(context_stack);
        }

        enum Action
        {
            PUSH,
            POP
        };

        Action action;
    };

    struct Instruction_IF : public Validated_Instruction
    {
        Instruction_IF(const Context& context, const std::vector<std::string>& arguments)
            : Validated_Instruction(".if", arguments)
        {
            evaluated_argument = evaluate_argument(context, arguments[0]);
        }

        void update_context_stack(ContextStack& context_stack) const override
        {
            context_stack.push();
            context_stack.get_current_context()->set_parsing_mode(
                    evaluated_argument ? Context::CONDITIONAL_TRUE : Context::CONDITIONAL_FALSE);
            InstructionAction::update_context_stack(context_stack);
        }

        int evaluated_argument;
    };

    struct Instruction_ELSE : public Instruction::InstructionAction
    {
        explicit Instruction_ELSE(const Context& context)
        {
            previous_mode = context.get_parsing_mode();

            if (previous_mode != Context::ParsingMode::CONDITIONAL_TRUE &&
                previous_mode != Context::ParsingMode::CONDITIONAL_FALSE)
            {
                throw InvalidElse();
            }
        }

        void update_context_stack(ContextStack& context_stack) const override
        {
            context_stack.get_current_context()->set_parsing_mode(
                    previous_mode == Context::CONDITIONAL_TRUE ? Context::CONDITIONAL_FALSE
                                                               : Context::CONDITIONAL_TRUE);
            InstructionAction::update_context_stack(context_stack);
        }

        Context::ParsingMode previous_mode;
    };

    struct Instruction_ENDIF : public Instruction::InstructionAction
    {
        explicit Instruction_ENDIF(const Context& context) {}
    };

    struct Instruction_EMPTY : public Instruction::InstructionAction
    {
    };
}

int Instruction::InstructionAction::evaluate_fixed_address(const Context& context,
                                                           int address) const
{
    return address;
}

int Instruction::InstructionAction::advance_address(const Context& context, int address) const
{
    return address;
}

void Instruction::InstructionAction::build(const Context& context, int address)
{
    // By default, nothing it built.
}

void Instruction::InstructionAction::write_bytes(const Context& context, ByteWriter& writer,
                                                 int address) const
{
    // By default, doesn't write anything.
}

void Instruction::InstructionAction::write_listing(Listing& listing, const std::string& input_line,
                                                   uint32_t line_number, int address) const
{
    listing.simple_line(line_number, input_line);
}

void Instruction::InstructionAction::update_context_stack(ContextStack& context_stack) const {}

InstructionEnum instruction_to_enum(std::string_view opcode)
{
    static std::vector<std::tuple<const char*, InstructionEnum>> association = {
            {"equ", InstructionEnum::EQU},        {"end", InstructionEnum::END},
            {"cpu", InstructionEnum::CPU},        {"org", InstructionEnum::ORG},
            {"data", InstructionEnum::DATA},      {".include", InstructionEnum::INCLUDE},
            {".syntax", InstructionEnum::SYNTAX}, {".context", InstructionEnum::CONTEXT},
            {".if", InstructionEnum::IF},         {".else", InstructionEnum::ELSE},
            {".endif", InstructionEnum::ENDIF}};
    if (opcode.empty())
    {
        return InstructionEnum::EMPTY;
    }
    auto found_op_code = std::ranges::find_if(association, [&opcode](const auto& t) {
        const auto& [opcode_str, opcode_enum] = t;
        return ci_equals(opcode, opcode_str);
    });

    if (found_op_code == association.end())
    {
        return InstructionEnum::OTHER;
    }

    return std::get<1>(*found_op_code);
}

Instruction::Instruction(const Context& context, const std::string& opcode,
                         const std::vector<std::string>& arguments, FileReader& file_reader)
{
    auto opcode_enum = instruction_to_enum(opcode);

    switch (opcode_enum)
    {
        case InstructionEnum::CPU:
            action = std::make_unique<Instruction_CPU>(arguments);
            break;
        case InstructionEnum::EMPTY:
            action = std::make_unique<Instruction_EMPTY>();
            break;
        case InstructionEnum::EQU:
            action = std::make_unique<Instruction_EQU>(arguments);
            break;
        case InstructionEnum::END:
            action = std::make_unique<Instruction_END>();
            break;
        case InstructionEnum::ORG:
            action = std::make_unique<Instruction_ORG>(context, arguments);
            break;
        case InstructionEnum::DATA:
            action = std::make_unique<Instruction_DATA>(context, arguments);
            break;
        case InstructionEnum::INCLUDE:
            action = std::make_unique<Instruction_INCLUDE>(context, arguments, file_reader);
            break;
        case InstructionEnum::SYNTAX:
            action = std::make_unique<Instruction_SYNTAX>(context, arguments);
            break;
        case InstructionEnum::CONTEXT:
            action = std::make_unique<Instruction_CONTEXT>(context, arguments);
            break;
        case InstructionEnum::OTHER:
            action = std::make_unique<Instruction_OTHER>(
                    context, opcode, arguments, context.get_options().new_syntax ? NEW : OLD);
            break;
        case InstructionEnum::IF:
            action = std::make_unique<Instruction_IF>(context, arguments);
            break;
        case InstructionEnum::ELSE:
            action = std::make_unique<Instruction_ELSE>(context);
            break;
        case InstructionEnum::ENDIF:
            action = std::make_unique<Instruction_ENDIF>(context);
            break;
        default:
            assert(0 && "Missing case in the Instruction Factory.");
    }
}

int Instruction::get_evaluation(const Context& context, int address) const
{
    return action->evaluate_fixed_address(context, address);
}

int Instruction::first_pass(ContextStack& context_stack, int address) const
{
    action->update_context_stack(context_stack);
    return action->advance_address(*context_stack.get_current_context(), address);
}

void Instruction::second_pass(const Context& context, ByteWriter& writer, const int address) const
{
    action->build(context, address);
    action->write_bytes(context, writer, address);
}

void Instruction::listing_pass(Listing& listing, const std::string& input_line,
                               uint32_t line_number, int address) const
{
    action->write_listing(listing, input_line, line_number, address);
}

InvalidCPU::InvalidCPU() { reason = R"(only allowed cpu is "8008" or "i8008")"; }

InvalidSyntax::InvalidSyntax() { reason = R"(only allowed syntax is "OLD" or "NEW")"; }

InvalidContextAction::InvalidContextAction()
{
    reason = R"(only allowed action is "PUSH" or "POP")";
}

InvalidElse::InvalidElse() { reason = R"(found .else without .if)"; }

MissingArgument::MissingArgument(std::string_view instruction)
{
    reason = "Missing parameter(s) for instruction: " + std::string{instruction};
}
