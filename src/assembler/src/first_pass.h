#ifndef INC_8008_ASSEMBLER_FIRST_PASS_H
#define INC_8008_ASSEMBLER_FIRST_PASS_H

#include "errors.h"

#include <vector>

class Context;
class ParsedLine;
class SymbolTable;

void first_pass(const Context& context, std::istream& input_stream, SymbolTable& symbol_table,
                std::vector<ParsedLine>& parsed_lines);

class AlreadyDefinedSymbol : public ExceptionWithReason
{
public:
    AlreadyDefinedSymbol(const std::string& symbol, int value);
};

#endif //INC_8008_ASSEMBLER_FIRST_PASS_H
