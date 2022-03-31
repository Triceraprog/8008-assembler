#ifndef INC_8008_ASSEMBLER_FIRST_PASS_H
#define INC_8008_ASSEMBLER_FIRST_PASS_H

#include "errors.h"

#include <vector>

class Context;
class FileReader;
class ParsedLine;
class SymbolTable;

void first_pass(Context& context, FileReader& file_reader, std::vector<ParsedLine>& parsed_lines);

class AlreadyDefinedSymbol : public ExceptionWithReason
{
public:
    AlreadyDefinedSymbol(const std::string& symbol, int value);
};

#endif //INC_8008_ASSEMBLER_FIRST_PASS_H
