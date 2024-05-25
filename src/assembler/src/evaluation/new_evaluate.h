#include "string_to_int.h"

#include <cctype>
#include <concepts>
#include <functional>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>

class Context;

namespace SimpleEvaluator
{
    struct ValueWasExpected : public std::exception
    {
    };

    namespace
    {
        using function_type = std::function<int(const int*)>;
    }

    struct Operation
    {
        int precedence;
        int arity;

        function_type function;
    };

    template<typename T>
    concept is_a_configuration = requires(T v) {
                                     {
                                         v.symbol_to_value("")
                                         } -> std::convertible_to<int>;
                                     {
                                         v.function_to_value("")
                                         } -> std::convertible_to<std::function<int(const int*)>>;
                                 };

    std::unordered_map<char, Operation>& get_intrinsic_binaries()
    {
        static std::unordered_map<char, Operation> intrinsic_binaries = {
                // Note: the args are presented in reverse order.
                {'+', Operation{1, 2, [](const int* args) { return args[1] + args[0]; }}},
                {'-', Operation{1, 2, [](const int* args) { return args[1] - args[0]; }}},
                {'*', Operation{2, 2, [](const int* args) { return args[1] * args[0]; }}},
                {'/', Operation{2, 2, [](const int* args) { return args[1] / args[0]; }}},
        };

        return intrinsic_binaries;
    }

    const Operation& get_intrinsic_binary(char token)
    {
        const auto& intrinsic_binaries = get_intrinsic_binaries();
        auto operation_it = intrinsic_binaries.find(token);

        if (operation_it != std::end(intrinsic_binaries))
        {
            return operation_it->second;
        }
        static Operation missing_operation{99, 2, [](const int* args) { return 0; }};
        return missing_operation;
    }

    int precedence(char token) { return get_intrinsic_binary(token).precedence; }

    std::unordered_map<char, Operation>& get_intrinsic_unaries()
    {
        static std::unordered_map<char, Operation> intrinsic_binaries = {
                {'+', Operation{99, 1, [](const int* args) { return args[0]; }}},
                {'-', Operation{99, 1, [](const int* args) { return -args[0]; }}},
        };

        return intrinsic_binaries;
    }

    Operation get_intrinsic_unary(char token)
    {
        const auto& intrinsic_unaries = get_intrinsic_unaries();
        auto operation_it = intrinsic_unaries.find(token);

        if (operation_it != std::end(intrinsic_unaries))
        {
            return operation_it->second;
        }
        static Operation missing_operation{99, 2, [](const int* args) { return 0; }};
        return missing_operation;
    }

    template<typename C>
        requires is_a_configuration<C>
    void pop_and_apply_op(std::stack<int>& values, std::stack<Operation>& operations,
                          const C& configuration)
    {
        const auto op = operations.top();
        const auto arity = op.arity;

        std::vector<int> args;

        for (auto i = 0; i < arity; i += 1)
        {
            if (values.empty())
            {
                throw ValueWasExpected{};
            }
            args.push_back(values.top());
            values.pop();
        }

        auto value = op.function(args.data());
        values.push(value);
        operations.pop();
    }

    bool is_valid_digit(char c)
    {
        return std::isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    }

    std::tuple<int, std::size_t> string_to_decimal(EvaluationFlags::Flags flags,
                                                   std::string_view tokens, std::size_t index)
    {
        // A bit convoluted...
        if (tokens[index] == '\'' && (index + 2) < tokens.length() && tokens[index + 2] == '\'')
        {
            const auto value = string_to_int(std::string{tokens.substr(index, 3)}, flags);
            return {value, index + 3};
        }

        const auto start_index = index;
        std::size_t size{};
        while (index < tokens.length() && is_valid_digit(tokens[index]))
        {
            index += 1;
            size += 1;
        }

        if (index < tokens.length() && size == 1)
        {
            const char c = tokens[index];
            if (c == 'x' || c == 'X')
            {
                index += 1;
                size += 1;
            }
        }

        while (index < tokens.length() && is_valid_digit(tokens[index]))
        {
            index += 1;
            size += 1;
        }

        if (index < tokens.length())
        {
            const char c = tokens[index];
            if (c == 'o' || c == 'b' || c == 'h')
            {
                index += 1;
                size += 1;
            }
        }

        int value = string_to_int(std::string{tokens.substr(start_index, size)}, flags);

        return {value, index};
    }

    std::tuple<std::string, std::size_t> string_to_symbol(std::string_view tokens,
                                                          std::size_t index)
    {
        const std::size_t start = index;
        const auto start_it = std::begin(tokens) + index;
        const auto first_not_alpha = std::find_if_not(start_it, std::end(tokens),
                                                      [](auto c) { return std::isalpha(c) || c == '_'; });
        const auto count = std::distance(start_it, first_not_alpha);
        std::string value{tokens.substr(start, count)};
        return {value, start + count};
    }

    bool is_suffix_operator(char token) { return token == '!'; }

    std::string get_infix_operators_as_string()
    {
        const auto& intrinsic_binaries = get_intrinsic_binaries();

        std::string tokens;
        tokens.reserve(intrinsic_binaries.size());

        for (const auto& pair : intrinsic_binaries)
        {
            tokens.push_back(pair.first);
        }

        return tokens;
    }

    bool is_infix_operator(char token)
    {
        static auto all_infix_tokens = get_infix_operators_as_string();
        return std::find(std::begin(all_infix_tokens), std::end(all_infix_tokens), token) !=
               std::end(all_infix_tokens);
    }

    bool in_unary_in_context(char token, char previous_token, std::size_t index)
    {
        if (is_suffix_operator(token))
        {
            return true;
        }

        if (token == '-' || token == '+')
        {
            if (index == 0)
            {
                return true;
            }
            else
            {
                return is_infix_operator(previous_token) || previous_token == '(';
            }
        }
        return false;
    }

    template<typename C>
        requires is_a_configuration<C>
    int evaluate(const C& configuration, EvaluationFlags::Flags flags, std::string_view tokens)
    {
        std::stack<int> values;
        std::stack<Operation> operations;

        std::size_t index = 0;
        char previous_token;

        while (index < tokens.length())
        {
            auto token = tokens[index];

            switch (token)
            {
                case ' ':
                    index += 1;
                    continue;
                case '(':
                    operations.push({0, -1, function_type{}});
                    break;
                case ')':
                    while (!operations.empty() && operations.top().arity != -1)
                    {
                        pop_and_apply_op(values, operations, configuration);
                    }
                    operations.pop();
                    break;
                default:
                    if (std::isdigit(token) || token == '\'')
                    {
                        auto [value, new_index] = string_to_decimal(flags, tokens, index);
                        values.push(value);
                        index = new_index - 1;
                    }
                    else if (std::isalpha(token))
                    {
                        auto [symbol, i] = string_to_symbol(tokens, index);
                        if (i < tokens.size() && tokens[i] == '(')
                        {
                            // This is a function.
                            // A function is  like a suffix operator acting on the evaluation
                            // of the following expression between parenthesis.
                            operations.push({99, 1, configuration.function_to_value(symbol)});
                        }
                        else
                        {
                            auto value = configuration.symbol_to_value(symbol);
                            values.push(value);
                        }
                        index = i - 1;
                    }
                    else
                    {
                        if (in_unary_in_context(token, previous_token, index))
                        {
                            operations.push(get_intrinsic_unary(token));
                            if (is_suffix_operator(token))
                            {
                                pop_and_apply_op(values, operations, configuration);
                            }
                        }
                        else
                        {
                            auto current_precedence = precedence(token);
                            while (!operations.empty() &&
                                   (operations.top().precedence) >= current_precedence)
                            {
                                pop_and_apply_op(values, operations, configuration);
                            }
                            operations.push(get_intrinsic_binary(token));
                        }
                    }
            }

            previous_token = token;
            index += 1;
        }

        while (!operations.empty())
        {
            pop_and_apply_op(values, operations, configuration);
        }
        return values.empty() ? 0 : values.top();
    }
}
