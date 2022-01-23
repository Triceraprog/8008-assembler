#include "evaluator.h"
#include "files.h"
#include "opcodes.h"
#include "options.h"
#include "symbol_table.h"
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
            std::cerr << "error: tried to read " << type_name << " '" << to_parse
                      << "'. Argument is invalid.\n";
            exit(-1);
        }
        catch (const std::out_of_range& e)
        {
            std::cerr << "error: tried to read " << type_name << " '" << to_parse
                      << "'. Argument is out of range.\n";
            exit(-1);
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

    int symbol_to_int(const SymbolTable& symbol_table, const std::basic_string<char>& to_parse)
    {
        int val;
        if (auto symbol_value = symbol_table.get_symbol_value(to_parse); std::get<0>(symbol_value))
        {
            val = std::get<1>(symbol_value);
        }
        else
        {
            std::cerr << "can't find symbol " << to_parse << "\n";
            exit(-1);
        }
        return val;
    }

    class Accumulator
    {
    public:
        void add_operation(char op) { operations.push_back(op); }
        void add_operand(std::string_view operand) { operands.emplace_back(operand); }

        int resolve(const Options& options, const SymbolTable& table, int current_line_count)
        {
            if (operands.size() != operations.size())
            {
                std::cerr << "line " << current_line_count << " the expression is ill-formed.\n";
            }

            int sum = 0;
            for (int j = 0; j < operands.size(); j++)
            {
                int val = 0;
                const auto& operand = operands[j];
                if (isalpha(operand.front()))
                {
                    val = symbol_to_int(table, operand);
                }
                else
                {
                    val = string_to_int(operand, EvaluationFlags::get_flags_from_options(options));
                }

                if (options.debug)
                {
                    std::cout << "      for %s got value " << operand;
                }

                {
                    const auto& op = operations[j];
                    if (op == '+')
                    {
                        sum += val;
                    }
                    else if (op == '-')
                    {
                        sum -= val;
                    }
                    else if (op == '*')
                    {
                        sum *= val;
                    }
                    else if (op == '/')
                    {
                        sum /= val;
                    }
                    else if (op == '#')
                    {
                        sum = sum * 256 + val;
                    }
                    else
                    {
                        std::cerr << "line " << current_line_count;
                        std::cerr << " unknown operation " << op << std::endl;
                        exit(-1);
                    }
                }
            }
            return sum;
        }

    private:
        std::vector<std::string> operands;
        std::vector<char> operations;
    };

}

int evaluate_argument(Options& options, const SymbolTable& symbol_table, int current_line_count,
                      std::string_view arg)
{
    if (arg.starts_with("\\HB\\"))
    {
        int value = evaluate_argument(options, symbol_table, current_line_count, arg.begin() + 4);
        return ((value >> 8) & 0xFF);
    }
    if (arg.starts_with("\\LB\\"))
    {
        int value = evaluate_argument(options, symbol_table, current_line_count, arg.begin() + 4);
        return (value & 0xFF);
    }

    if (options.debug)
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
            std::cerr << "Error, expected value, found: " << arg_to_parse << std::endl;
            exit(-1);
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
                    std::cerr << "Error '" << op << "'" << std::endl;
                    exit(-1);
            }
        }
    }

    auto result = acc.resolve(options, symbol_table, current_line_count);

    if (options.debug)
    {
        std::cout << "     for got sum " << result << "\n";
    }

    return result;
}

EvaluationFlags::Flags EvaluationFlags::get_flags_from_options(const Options& options)
{
    return options.input_num_as_octal ? ThreeDigitsAsOctal : None;
}
