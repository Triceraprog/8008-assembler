#include "string_to_int.h"
#include "evaluate.h"
#include "legacy_evaluate.h"
#include "options.h"
#include <iostream>

EvaluationFlags::Flags EvaluationFlags::get_flags_from_options(const Options& options)
{
    return options.input_num_as_octal ? ThreeDigitsAsOctal : None;
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

int string_to_int(const std::string& to_parse, EvaluationFlags::Flags flags)
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
