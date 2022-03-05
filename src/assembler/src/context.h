#ifndef INC_8008_ASSEMBLER_CONTEXT_H
#define INC_8008_ASSEMBLER_CONTEXT_H

class Options;
class SymbolTable;

struct Context
{
    const Options& options;
    SymbolTable& symbolTable;
};

#endif //INC_8008_ASSEMBLER_CONTEXT_H
