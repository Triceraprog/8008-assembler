#include "data_extraction.h"
#include "evaluator.h"
#include "options.h"
#include "symbol_table.h"
#include <algorithm>

int decode_data(const Context& context, const std::vector<std::string>& tokens,
                std::vector<int>& out_data)
{
    if (tokens.empty())
    {
        throw EmptyData();
    }

    auto first_argument = tokens.front();
    if (first_argument.front() == '*')
    {
        // 'DATA *NNN' reserve NNN bytes.
        // int number_to_reserve = std::stoi(data_part.substr(1).data());
        const auto first_comment = first_argument.find_first_of(';');
        auto without_comment = std::string{first_argument.substr(0, first_comment)};
        int number_to_reserve = evaluate_argument(context, without_comment.substr(1));
        return 0 - number_to_reserve;
    }
    if ((first_argument.front() == '\'') || (first_argument.front() == '"'))
    {
        // DATA "..." or DATA '...' declare strings of characters
        // The argument has already been extracted (by the LineTokenizer), so it is assured
        // to be a quoted string.
        // It is possible that the string is open-ended on the right (no closing quote)
        const char quote_to_find = first_argument.front();
        auto last_quote_position = first_argument.back() == quote_to_find
                                           ? first_argument.length() - 1
                                           : first_argument.length();
        auto string_content = first_argument.substr(1, last_quote_position - 1);

        bool escape_char = false;

        for (char char_data : string_content)
        {
            if (escape_char)
            {
                escape_char = false;
                switch (char_data)
                {
                    case '\\':
                        out_data.push_back('\\');
                        break;
                    case 'r':
                        out_data.push_back('\r');
                        break;
                    case 'n':
                        out_data.push_back('\n');
                        break;
                    case 't':
                        out_data.push_back('\t');
                        break;
                    case '0':
                        out_data.push_back('\0');
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
                out_data.push_back(char_data);
            }
        }

        /* If "markascii" option is set, the highest bit of these ascii bytes are forced to 1. */
        if (context.get_options().mark_8_ascii)
        {
            std::ranges::transform(out_data.begin(), out_data.end(), out_data.begin(),
                                   [](const auto p) { return p | 0x80; });
        }

        return static_cast<int>(out_data.size());
    }
    else
    {
        /* DATA xxx,xxx,xxx,xxx */
        int byte_count = 0;

        for (const auto& argument : tokens)
        {
            out_data.push_back(evaluate_argument(context, argument));
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

EmptyData::EmptyData() { reason = "DATA has no parameter."; }
