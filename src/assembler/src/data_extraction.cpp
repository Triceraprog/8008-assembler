#include "data_extraction.h"
#include "evaluator.h"
#include "options.h"
#include "symbol_table.h"
#include <cstdlib>
#include <iostream>
#include <regex>

namespace
{
    std::regex data_rule{"[dD][aA][tT][aA]\\s*"};
    std::regex except_comma{R"(([^,\s]*))"};
}

int decode_data(const Options& options, const SymbolTable& symbol_table,
                const std::string_view line, int* out_data)
{
    std::string line_as_string{line};
    std::smatch data_match;
    bool found = std::regex_search(line_as_string, data_match, data_rule);

    if (!found)
    {
        throw InternalError("can't find data code");
    }

    auto after_data_position = data_match.position() + data_match.length();
    auto data_part = line.substr(after_data_position);

    if (data_part.empty())
    {
        return 0;
    }

    if (data_part.front() == '*')
    {
        // 'DATA *NNN' reserve NNN bytes.
        try
        {
            int number_to_reserve = std::stoi(data_part.substr(1).data());
            return 0 - number_to_reserve;
        }
        catch (...)
        {
            std::cerr << " in line %s can't read number to reserve\n";
            exit(-1);
        }
    }
    if ((data_part.front() == '\'') || (data_part.front() == '"'))
    {
        // DATA "..." or DATA '...' declares a string of characters
        // Warning: there's a syntax limitation. If a comment contains a quote, then
        // the result will be wrong.
        const char quote_to_find = data_part.front();
        auto last_quote_position = data_part.find_last_of(quote_to_find);
        auto string_content = data_part.substr(1, last_quote_position - 1);

        bool escape_char = false;
        int* out_pointer = out_data;

        for (char char_data : string_content)
        {
            if (escape_char)
            {
                escape_char = false;
                switch (char_data)
                {
                    case '\\':
                        *(out_pointer++) = '\\';
                        break;
                    case 'r':
                        *(out_pointer++) = '\r';
                        break;
                    case 'n':
                        *(out_pointer++) = '\n';
                        break;
                    case 't':
                        *(out_pointer++) = '\t';
                        break;
                    case '0':
                        *(out_pointer++) = '\0';
                        break;
                    default:
                        throw UnknownEscapeSequence(char_data);
                }
            }
            else if (char_data == '\\')
            {
                escape_char = true;
            }
            else
            {
                *(out_pointer++) = static_cast<u_char>(char_data);
            }
        }

        /* If "markascii" option is set, highest bit of these ascii bytes are forced to 1. */
        if (options.mark_8_ascii)
        {
            for (auto* p = out_data; p < out_pointer; p++)
            {
                *p |= 0x80;
            }
        }

        return static_cast<int>(out_pointer - out_data);
    }
    else
    {
        /* DATA xxx,xxx,xxx,xxx */
        const auto first_comment = data_part.find_first_of(';');
        auto without_comment = std::string{data_part.substr(0, first_comment)};

        int byte_count = 0;

        auto begin =
                std::sregex_iterator(without_comment.begin(), without_comment.end(), except_comma);
        auto end = std::sregex_iterator();

        for (auto it = begin; it != end; it++)
        {
            std::string sub = it->str();

            if (sub.empty())
            {
                continue;
            }

            *(out_data++) = evaluate_argument(options, symbol_table, sub);

            byte_count += 1;

            if (byte_count > 12)
            {
                throw DataTooLong();
            }
        }

        return byte_count;
    }
}

DataTooLong::DataTooLong()
{
    reason = "DATA max length is 12 bytes. Use a second line for more data.";
}

UnknownEscapeSequence::UnknownEscapeSequence(char escape)
{
    reason = "unknown escape sequence \\" + std::string(1, escape);
}
