#include "legacy_evaluate.h"

#include "context.h"
#include "evaluate.h"
#include "opcodes/opcodes.h"
#include "options.h"
#include "string_to_int.h"
#include "utils.h"

#include <iostream>
#include <regex>

std::regex not_an_operator{R"(^\s*([^\+\*-/#\s]+)\s*)"};

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
    if ((isalpha(front) || front == '_') && front != '\'')
    {
        return symbol_to_int(context, operand);
    }
    else
    {
        return string_to_int(operand,
                             EvaluationFlags::get_flags_from_options(context.get_options()));
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

            if (context.get_options().debug)
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

int legacy_evaluator(const Context& context, std::string_view arg)
{
    Accumulator acc;
    acc.add_operation('+'); // First operation is to add to the accumulator being 0.

    std::string_view arg_to_parse{arg};
    while (!arg_to_parse.empty())
    {
        // Quickly parse quoted char
        if (arg_to_parse[0] == '\'' && arg_to_parse.size() >= 3 && arg_to_parse[2] == '\'')
        {
            acc.add_operand(arg_to_parse.substr(0, 3));
            arg_to_parse = arg_to_parse.substr(3);
        }
        else
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
            arg_to_parse = arg_to_parse.substr(data_match.length());
        }

        // Parse Operator
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
    return result;
}
