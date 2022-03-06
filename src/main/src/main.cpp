#include "assembler/src/context.h"
#include "assembler/src/errors.h"
#include "assembler/src/files.h"
#include "assembler/src/first_pass.h"
#include "assembler/src/listing.h"
#include "assembler/src/options.h"
#include "assembler/src/parsed_line.h"
#include "assembler/src/second_pass.h"
#include "assembler/src/symbol_table.h"
#include "listing_pass.h"

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
        std::vector<ParsedLine> parsed_lines;

        Context context{global_options, symbol_table};

        first_pass(context, files, symbol_table, parsed_lines);
        second_pass(context, files, symbol_table, parsed_lines);
        listing_pass(context, files, parsed_lines, listing);

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
