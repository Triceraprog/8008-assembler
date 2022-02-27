#ifndef INC_8008_ASSEMBLER_FIRST_PASS_H
#define INC_8008_ASSEMBLER_FIRST_PASS_H

#include "errors.h"

#include <vector>

class SymbolTable;
class Files;
class Options;
class Listing;
class ParsedLine;

void first_pass(const Options& options, SymbolTable& symbol_table, Files& files,
                std::vector<ParsedLine>& parsed_lines, Listing& listing);

class AlreadyDefinedSymbol : public ExceptionWithReason
{
public:
    AlreadyDefinedSymbol(const std::string& symbol, int value);
};

class InvalidCPU : public ExceptionWithReason
{
public:
    InvalidCPU();
};

#endif //INC_8008_ASSEMBLER_FIRST_PASS_H
