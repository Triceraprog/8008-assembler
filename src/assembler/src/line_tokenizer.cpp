#include "line_tokenizer.h"
#include "options.h"
#include "utils.h"

#include <iostream>
#include <regex>

namespace
{
    std::regex line_tokens_scan{R"(([^\s,]+))"};
}

LineTokenizer::LineTokenizer(const std::string& line)
{
    if (line.empty())
    {
        return;
    }

    auto c = line[0];
    auto used_first_column = (c != ' ') && (c != '\t') && (c != 0x00);

    auto begin = std::sregex_iterator(line.begin(), line.end(), line_tokens_scan);
    auto end = std::sregex_iterator();

    std::deque<std::string> parsed;
    std::ranges::transform(begin, end, std::back_inserter(parsed),
                           [](const auto& elt) { return elt.str(); });
    auto arg_count = std::max(0, static_cast<int>(parsed.size()) - (used_first_column ? 2 : 1));
    arguments.reserve(arg_count);

    if (used_first_column)
    {
        label = consume_parsed(parsed);
        adjust_label();
    }
    opcode = consume_parsed(parsed);
    while (arg_count > 0)
    {
        arguments.push_back(consume_parsed(parsed));
        arg_count -= 1;
    }
}

std::string LineTokenizer::consume_parsed(std::deque<std::string>& parsed)
{
    if (parsed.empty())
    {
        return "";
    }
    else
    {
        std::string result = parsed.front();
        parsed.pop_front();
        return result;
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

        std::ranges::replace_if(
                clean, [](const auto c) { return c == ','; }, ' ');

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