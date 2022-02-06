#ifndef INC_8008_ASSEMBLER_FIRST_PASS_H
#define INC_8008_ASSEMBLER_FIRST_PASS_H

#include "errors.h"

class SymbolTable;
class Files;
class Options;

void first_pass(const Options& options, SymbolTable& symbol_table, Files& files);

class AlreadyDefinedSymbol : public ExceptionWithReason
{
public:
    AlreadyDefinedSymbol(const std::string& symbol, int value);
};

#endif //INC_8008_ASSEMBLER_FIRST_PASS_H
