#include "assembler/src/files.h"
#include "assembler/src/first_pass.h"
#include "assembler/src/options.h"
#include "assembler/src/second_pass.h"
#include "assembler/src/symbol_table.h"

int main(int argc, const char** argv)
{
    Options global_options;

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

    first_pass(global_options, symbol_table, files);
    second_pass(global_options, symbol_table, files);

    /* write symbol table to listfile */
    if (global_options.generate_list_file)
    {
        symbol_table.list_symbols(files.lfp);
    }
}

