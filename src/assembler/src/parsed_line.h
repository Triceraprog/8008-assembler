#ifndef INC_8008_ASSEMBLER_PARSED_LINE_H
#define INC_8008_ASSEMBLER_PARSED_LINE_H

#include "line_tokenizer.h"
#include "instruction.h"

#include <memory>

struct ParsedLine
{
    int line_number;
    int line_address;
    LineTokenizer tokens;
    Instruction instruction;
    std::string line;
};

#endif //INC_8008_ASSEMBLER_PARSED_LINE_H
