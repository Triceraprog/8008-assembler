#include "evaluator.h"
#include "context.h"
#include "opcodes/opcodes.h"
#include "options.h"
#include "utils.h"

#include <iostream>
#include <regex>

namespace
{
    std::regex not_an_operator{R"(^\s*([^\+\*-/#\s]+)\s*)"};

    namespace EvaluationFlags
    {
        enum Flags
        {
            None = 0,
            ThreeDigitsAsOctal = 1
        };

        Flags get_flags_from_options(const Options& options);
    }

    int parse_number_value(const std::string& to_parse, int base, std::string_view type_name)
    {
        int val;
        try
        {
            val = std::stoi(to_parse, nullptr, base);
        }
        catch (const std::invalid_argument& e)
        {
            throw InvalidNumber(to_parse, type_name, "invalid");
        }
        catch (const std::out_of_range& e)
        {
            throw InvalidNumber(to_parse, type_name, "out of range");
        }
        return val;
    }

    int string_to_int(const std::basic_string<char>& to_parse, EvaluationFlags::Flags flags)
    {
        int val;
        const auto last_char_in_parsing = tolower(to_parse.back());
        if (last_char_in_parsing == 'o')
        {
            val = parse_number_value(to_parse, 8, "octal");
        }
        else if (last_char_in_parsing == 'h')
        {
            val = parse_number_value(to_parse, 16, "hex");
        }
        else if (to_parse.starts_with("0x") || to_parse.starts_with("0X"))
        {
            val = parse_number_value(to_parse.substr(2), 16, "hex");
        }
        else if (last_char_in_parsing == 'b')
        {
            val = parse_number_value(to_parse, 2, "binary");
        }
        else if (last_char_in_parsing == '\'' && to_parse.front() == '\'' && to_parse.size() == 3)
        {
            val = static_cast<unsigned char>(to_parse[1]);
        }
        else
        {
            if ((to_parse.size() == 3) && (flags == EvaluationFlags::ThreeDigitsAsOctal))
            {
                val = parse_number_value(to_parse, 8, "octal");
            }
            else
            {
                val = parse_number_value(to_parse, 10, "decimal");
            }
        }
        return val;
    }

    int symbol_to_int(const Context& context, const std::basic_string<char>& to_parse)
    {
        if (auto symbol_value = context.get_symbol_value(to_parse); std::get<0>(symbol_value))
        {
            return std::get<1>(symbol_value);
        }
        throw CannotFindSymbol(to_parse);
    }

    int operand_to_int(const Context& context, const std::string& operand)
    {
        auto front = operand.front();
        if (isalpha(front) && front != '\'')
        {
            return symbol_to_int(context, operand);
        }
        else
        {
            return string_to_int(operand, EvaluationFlags::get_flags_from_options(context.options));
        }
    }

    int apply_operation(const char& op, int acc, int val)
    {
        switch (op)
        {
            case '+':
                acc += val;
                break;
            case '-':
                acc -= val;
                break;
            case '*':
                acc *= val;
                break;
            case '/':
                acc /= val;
                break;
            case '#':
                acc = acc * 256 + val;
                break;
            default:
                throw UnknownOperation(op);
        }
        return acc;
    }

    class Accumulator
    {
    public:
        void add_operation(char op) { operations.push_back(op); }
        void add_operand(std::string_view operand) { operands.emplace_back(operand); }

        [[nodiscard]] int resolve(const Context& context) const
        {
            if (operands.size() != operations.size())
            {
                throw IllFormedExpression();
            }

            int sum = 0;
            for (int j = 0; j < operands.size(); j++)
            {
                const auto& operand = operands[j];
                int val = operand_to_int(context, operand);

                if (context.options.debug)
                {
                    std::cout << "      for '" << operand << "' got value " << val;
                }

                {
                    const auto& op = operations[j];
                    sum = apply_operation(op, sum, val);
                }
            }
            return sum;
        }

    private:
        std::vector<std::string> operands;
        std::vector<char> operations;
    };

}

int evaluate_argument(const Context& context, std::string_view arg)
{
    if (arg.starts_with("\\HB\\"))
    {
        int value = evaluate_argument(context, arg.begin() + 4);
        return ((value >> 8) & 0xFF);
    }
    if (arg.starts_with("H(") && arg.ends_with(')'))
    {
        int value = evaluate_argument(context, arg.substr(2, arg.size() - 2 - 1));
        return ((value >> 8) & 0xFF);
    }

    if (arg.starts_with("\\LB\\"))
    {
        int value = evaluate_argument(context, arg.begin() + 4);
        return (value & 0xFF);
    }
    if (arg.starts_with("L(") && arg.ends_with(')'))
    {
        int value = evaluate_argument(context, arg.substr(2, arg.size() - 2 - 1));
        return (value & 0xFF);
    }

    if (context.options.debug)
    {
        std::cout << "evaluating " << arg << "\n";
    }

    Accumulator acc;
    acc.add_operation('+'); // First operation is to add to the accumulator being 0.

    std::string_view arg_to_parse{arg};
    while (!arg_to_parse.empty())
    {
        // Parse Operand
        std::string value_to_parse(arg_to_parse);
        std::smatch data_match;
        bool found = std::regex_search(value_to_parse, data_match, not_an_operator);

        if (!found)
        {
            throw ExpectedValue(arg_to_parse);
        }

        acc.add_operand(trim_string(data_match.str()));

        // Parse Operator
        arg_to_parse = arg_to_parse.substr(data_match.length());
        if (!arg_to_parse.empty())
        {
            char op = arg_to_parse.front();

            switch (op)
            {
                case '+':
                case '-':
                case '/':
                case '*':
                case '#':
                    acc.add_operation(op);
                    arg_to_parse = arg_to_parse.substr(1);
                    break;
                default:
                    throw UnknownOperation(op);
            }
        }
    }

    auto result = acc.resolve(context);

    if (context.options.debug)
    {
        std::cout << "     got final value " << result << "\n";
    }

    return result;
}

EvaluationFlags::Flags EvaluationFlags::get_flags_from_options(const Options& options)
{
    return options.input_num_as_octal ? ThreeDigitsAsOctal : None;
}

CannotFindSymbol::CannotFindSymbol(const std::string& symbol)
{
    reason = "cannot find symbol " + symbol;
}

UnknownOperation::UnknownOperation(const char operation)
{
    reason = "unknown operation ";
    reason.append(1, operation);
}

IllFormedExpression::IllFormedExpression() { reason = "the expression is ill-formed"; }

InvalidNumber::InvalidNumber(const std::string_view to_parse, std::string_view type_name,
                             std::string_view fail_reason)
{
    reason = "Tried to read '" + std::string{to_parse} + "' as " + std::string{type_name} +
             ". Argument is " + std::string{fail_reason};
}

ExpectedValue::ExpectedValue(std::string_view to_parse)
{
    reason = "Expected value, found '" + std::string(to_parse) + "'";
}
