#include "line_tokenizer.h"
#include "options.h"
#include "utils.h"

#include <iostream>
#include <utility>

namespace
{
    struct LineParser
    {
        explicit LineParser(std::string view) : view{std::move(view)} {}

        std::string next_string()
        {
            skip_spaces();

            if (is_comment())
            {
                return consume_full_view();
            }

            auto first_space = view.find_first_of(" \t;");
            if (first_space == std::string::npos)
            {
                return consume_full_view();
            }

            auto result = std::string{view.substr(0, first_space)};
            view = view.substr(first_space + 1);
            return result;
        }

        std::string next_argument()
        {
            skip_spaces();
            if ((view.front() == '\'') || (view.front() == '"'))
            {
                return next_quoted_string();
            }

            auto first_comma = view.find_first_of(",;");
            if (first_comma == std::string::npos)
            {
                return consume_full_view();
            }
            auto result = view.substr(0, first_comma);
            view = view.substr(first_comma + 1);
            return result;
        }

        [[nodiscard]] bool empty() const { return view.empty(); }

    private:
        [[nodiscard]] bool is_comment() const { return view.front() == ';'; }

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
                    auto result = view.substr(0, index + 1);
                    view = view.substr(std::min(index + 2, view.size()));
                    return result;
                }
            }
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

        std::string view;
    };
}

LineTokenizer::LineTokenizer(const std::string& line)
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
        auto next = line_parser.next_string();

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
        auto next = line_parser.next_string();

        if (next.front() == ';')
        {
            comment = std::move(next);
            return;
        }
        opcode = std::move(next);
    }

    while (!line_parser.empty())
    {
        arguments.push_back(line_parser.next_argument());
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
    std::string clean_line(const std::string& input_line)
    {
        std::string clean{input_line};

        auto pos = clean.find_first_of(";/\n\x0a");
        if (pos != std::string::npos)
        {
            clean.resize(std::max(0, static_cast<int>(pos - 1)));
        }

        return clean;
    }

}

LineTokenizer parse_line(const Options& options, const std::string& line, int line_count)
{
    std::string cleaned_line = clean_line(line);

    LineTokenizer tokens(cleaned_line);

    if (tokens.warning_on_label)
    {
        std::cerr << "WARNING: in line " << line_count << " " << cleaned_line << " label "
                  << tokens.label << " lacking colon, and not 'equ' pseudo-op.\n";
    }

    if ((tokens.arguments.size() > 2) && (!ci_equals(tokens.opcode, "data")))
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
    }

    return tokens;
}