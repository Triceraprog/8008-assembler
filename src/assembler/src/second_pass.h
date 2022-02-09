#ifndef INC_8008_ASSEMBLER_SECOND_PASS_H
#define INC_8008_ASSEMBLER_SECOND_PASS_H

#include "errors.h"

class Options;
class SymbolTable;
class Files;
class Listing;

void second_pass(const Options& options, const SymbolTable& symbol_table, Files& files,
                 Listing& listing);

class ExpectedArgumentWithinLimits : public ExceptionWithReason
{
public:
    ExpectedArgumentWithinLimits(int limit, std::string& content, int evaluated);
};

class UnexpectedArgumentCount : public ExceptionWithReason
{
public:
    explicit UnexpectedArgumentCount(uint32_t arg_count);
};

#endif //INC_8008_ASSEMBLER_SECOND_PASS_H
