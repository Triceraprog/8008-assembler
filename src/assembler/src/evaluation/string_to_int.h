#ifndef INC_8008_ASSEMBLER_STRING_TO_INT_H
#define INC_8008_ASSEMBLER_STRING_TO_INT_H

#include <string>

class Options;

namespace EvaluationFlags
{
    enum Flags
    {
        None = 0,
        ThreeDigitsAsOctal = 1
    };

    Flags get_flags_from_options(const Options& options);
}

int string_to_int(const std::string& to_parse, EvaluationFlags::Flags flags);

#endif //INC_8008_ASSEMBLER_STRING_TO_INT_H
