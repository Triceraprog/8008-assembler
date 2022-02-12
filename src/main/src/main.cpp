#include "assembler/src/errors.h"
#include "assembler/src/files.h"
#include "assembler/src/first_pass.h"
#include "assembler/src/options.h"
#include "assembler/src/second_pass.h"
#include "assembler/src/symbol_table.h"
#include "assembler/src/listing.h"

#include <iostream>

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

    try
    {
        Files files(global_options);
        SymbolTable symbol_table;
        Listing listing(files.listing_stream, global_options);

        first_pass(global_options, symbol_table, files, listing);
        second_pass(global_options, symbol_table, files, listing);

        /* write symbol table to listfile */
        if (global_options.generate_list_file)
        {
            symbol_table.list_symbols(files.listing_stream);
        }
    }
    catch (const CannotOpenFile& ex)
    {
        std::cerr << ex.what() << std::endl;
        exit(-1);
    }
    catch (const ParsingException& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(-1);
    }
}
