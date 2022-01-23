#include <iostream>

#include "assembler/src/options.h"
#include "assembler/src/symbol_table.h"
#include "assembler/src/files.h"

extern Options global_options;
extern void first_pass(SymbolTable& symbol_table, Files& files);
extern void second_pass(const SymbolTable& symbol_table, Files& files);
extern void writebyte(int data, int address, FILE* ofp);

int main(int argc, const char** argv)
{
    try
    {
        global_options.parse(argc, argv);
    }
    catch (InvalidCommandLine&)
    {
        exit(-1);
    }

    Files files(global_options);
    SymbolTable symbol_table;

    first_pass(symbol_table, files);
    second_pass(symbol_table, files);

    /* write symbol table to listfile */
    if (global_options.generate_list_file)
    {
        symbol_table.list_symbols(files.lfp);
    }
}
