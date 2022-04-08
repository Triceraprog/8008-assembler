#ifndef INC_8008_ASSEMBLER_PARSED_LINE_H
#define INC_8008_ASSEMBLER_PARSED_LINE_H

#include "instruction.h"
#include "line_tokenizer.h"

#include <memory>

class Context;

struct ParsedLine
{
    std::size_t line_number;
    int line_address;
    LineTokenizer tokens;
    Instruction instruction;
    std::string line;
    std::shared_ptr<std::string> name_tag;
    std::shared_ptr<Context> context;
};

#endif //INC_8008_ASSEMBLER_PARSED_LINE_H
