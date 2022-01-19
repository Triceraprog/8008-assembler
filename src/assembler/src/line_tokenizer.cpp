#include "line_tokenizer.h"
#include "utils.h"

#include <iostream>
#include <regex>

namespace
{
    std::regex label_scan{R"(([\w|\\#]+))"};
}

LineTokenizer::LineTokenizer(const std::string& line)
{
    if (line.empty())
    {
        return;
    }

    auto c = line[0];
    auto used_first_column = (c != ' ') && (c != '\t') && (c != 0x00);

    auto begin = std::sregex_iterator(line.begin(), line.end(), label_scan);
    auto end = std::sregex_iterator();

    std::deque<std::string> parsed;
    std::ranges::transform(begin, end, std::back_inserter(parsed),
                           [](const auto& elt) { return elt.str(); });
    arg_count = std::max(0, static_cast<int>(parsed.size()) - (used_first_column ? 2 : 1));

    if (used_first_column)
    {
        label = consume_parsed(parsed);
        adjust_label();
    }
    opcode = consume_parsed(parsed);
    arg1 = consume_parsed(parsed);
    arg2 = consume_parsed(parsed);
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

void LineTokenizer::debug_print() const
{
    std::cout << "parsed line label=" << label << " opcode=" << opcode << " arg1str=" << arg1
              << "\n";
}
