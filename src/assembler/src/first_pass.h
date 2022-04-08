#ifndef INC_8008_ASSEMBLER_FIRST_PASS_H
#define INC_8008_ASSEMBLER_FIRST_PASS_H

#include "errors.h"
#include "context_stack.h"

#include <vector>

class Context;
class FileReader;
class ParsedLine;
class SymbolTable;
class ParsedLineStorage;

void first_pass(ContextStack context_stack, FileReader& file_reader, ParsedLineStorage& parsed_line_storage);

class AlreadyDefinedSymbol : public ExceptionWithReason
{
public:
    AlreadyDefinedSymbol(const std::string& symbol, int value);
};

#endif //INC_8008_ASSEMBLER_FIRST_PASS_H
