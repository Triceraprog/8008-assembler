#ifndef INC_8008_ASSEMBLER_FIRST_PASS_H
#define INC_8008_ASSEMBLER_FIRST_PASS_H

#include "errors.h"

class SymbolTable;
class Files;
class Options;
class Listing;

void first_pass(const Options& options, SymbolTable& symbol_table, Files& files, Listing & listing);

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

class UndefinedOpcode : public ExceptionWithReason
{
public:
    UndefinedOpcode(const std::string& opcode);
};

#endif //INC_8008_ASSEMBLER_FIRST_PASS_H
