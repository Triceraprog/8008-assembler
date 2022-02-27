#ifndef INC_8008_ASSEMBLER_PARSED_LINE_H
#define INC_8008_ASSEMBLER_PARSED_LINE_H

#include "line_tokenizer.h"

#include <memory>

struct ParsedLine
{
    int line_number;
    int line_address;
    LineTokenizer tokens;
    std::string line;
};

#endif //INC_8008_ASSEMBLER_PARSED_LINE_H
