#ifndef INC_8008_ASSEMBLER_CONTEXT_H
#define INC_8008_ASSEMBLER_CONTEXT_H

class Options;
class SymbolTable;

struct Context
{
    Options& options;
    SymbolTable& symbol_table;
};

#endif //INC_8008_ASSEMBLER_CONTEXT_H
