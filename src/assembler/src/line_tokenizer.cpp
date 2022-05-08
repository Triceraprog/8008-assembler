#include "line_tokenizer.h"
#include "options.h"
#include "utils.h"

#include <cassert>
#include <iostream>
#include <utility>

namespace
{
    struct LineParser
    {
        explicit LineParser(std::string_view view) : view{view} {}

        std::string next_word() { return next_with_delimiters(" \t;"); }
        std::string next_argument() { return next_with_delimiters(",;"); }
        [[nodiscard]] bool empty() const { return view.empty(); }

    private:
        [[nodiscard]] bool is_comment() const { return view.front() == ';'; }

        std::string next_with_delimiters(std::string_view delimiters)
        {
            skip_spaces();
            if (is_comment())
            {
                return consume_full_view();
            }

            if ((view.front() == '\'') || (view.front() == '"'))
            {
                return next_quoted_string();
            }

            auto first_delimiter = view.find_first_of(delimiters);
            if (first_delimiter == std::string::npos)
            {
                return consume_full_view();
            }
            if (view[first_delimiter] == ';')
            {
                return consume_view_and_keep_next(first_delimiter);
            }
            return consume_view_and_skip_next(first_delimiter);
        }

        std::string next_quoted_string()
        {
            const char quote_type = view.front();
            for (std::size_t index = 1; index < view.length(); index += 1)
            {
                if (view[index] == '\\')
                {
                    index += 1; // Quoted char, skip next char
                }
                else if (view[index] == quote_type)
                {
                    return consume_view_and_skip_next(index + 1); // End of quoted string
                }
            }
            // We don't care about the presence of the closing quote if it's the end of the string
            return consume_full_view();
        }

        void skip_spaces()
        {
            auto first_not_space = view.find_first_not_of(" \t");
            if (first_not_space != std::string::npos)
            {
                view = view.substr(first_not_space);
            }
            else
            {
                view.resize(0);
            }
        }

        std::string consume_full_view()
        {
            auto result = view;
            view.resize(0);
            return std::string{trim_string(result)};
        }

        std::string consume_view_and_skip_next(std::size_t length_to_consume)
        {
            auto result = view.substr(0, length_to_consume);
            view = view.substr(std::min(length_to_consume + 1, view.size()));
            return std::string{trim_string(result)};
        }

        std::string consume_view_and_keep_next(std::size_t length_to_consume)
        {
            assert(length_to_consume > 0);
            auto result = view.substr(0, length_to_consume);
            view = view.substr(std::min(length_to_consume, view.size()));
            return std::string{trim_string(result)};
        }

        std::string view;
    };
}

LineTokenizer::LineTokenizer(const std::string_view line)
{
    if (line.empty())
    {
        return;
    }

    auto first_char = line[0];
    auto used_first_column = (first_char != ' ') && (first_char != '\t') && (first_char != 0x00);

    LineParser line_parser{line};

    if (used_first_column)
    {
        auto next = line_parser.next_word();

        if (next.front() == ';')
        {
            comment = std::move(next);
            return;
        }
        label = std::move(next);
        adjust_label();
    }

    if (!line_parser.empty())
    {
        auto next = line_parser.next_word();

        if (next.front() == ';')
        {
            comment = std::move(next);
            return;
        }
        opcode = std::move(next);
    }

    while (!line_parser.empty())
    {
        auto next = line_parser.next_argument();

        if (next.front() == ';')
        {
            comment = std::move(next);
            return;
        }
        if (!next.empty())
        {
            arguments.push_back(std::move(next));
        }
    }
}

void LineTokenizer::adjust_label()
{
    const char last_label_char = label.back();
    if (last_label_char == ':' || last_label_char == ',')
    {
        label.resize(label.size() - 1);
    }
    else if (ci_equals(label, "equ"))
    {
        warning_on_label = true;
    }
}

namespace
{
    bool is_data_opcode(std::string_view opcode) { return ci_equals(opcode, "data"); }
    bool is_extended_command(std::string_view opcode) { return opcode[0] == '.'; }
}

LineTokenizer parse_line(const Options& options, const std::string_view line,
                         std::size_t line_count)
{
    LineTokenizer tokens(line);

    if (tokens.warning_on_label)
    {
        std::cerr << "WARNING: in line " << line_count << " " << line << " label " << tokens.label
                  << " lacking colon, and not 'equ' pseudo-op.\n";
    }

    if ((tokens.arguments.size() > 2) &&
        (!is_data_opcode(tokens.opcode) && !is_extended_command(tokens.opcode)))
    {
        std::cerr << "WARNING: extra text on line " << line_count << " " << line << "\n";
    }

    if (options.debug)
    {
        auto arg_count = tokens.arguments.size();
        std::cout << "label=<" << tokens.label << "> ";
        std::cout << "opcode=<" << tokens.opcode << "> ";
        std::cout << "args=<" << arg_count << "> ";

        for (int index = 0; index < arg_count; index += 1)
        {
            std::cout << "arg" << index << "=<" << tokens.arguments[index] << "> ";
        }

        std::cout << "\n";
    }

    return tokens;
}