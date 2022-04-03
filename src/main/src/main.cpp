#include "assembler/src/context.h"
#include "assembler/src/errors.h"
#include "assembler/src/files/files.h"
#include "assembler/src/first_pass.h"
#include "assembler/src/listing.h"
#include "assembler/src/listing_pass.h"
#include "assembler/src/options.h"
#include "assembler/src/parsed_line_storage.h"
#include "assembler/src/second_pass.h"
#include "assembler/src/symbol_table.h"

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
        ParsedLineStorage parsed_line_storage;

        Context context{global_options, symbol_table};

        first_pass(context, files.file_reader, parsed_line_storage);
        second_pass(context, files.output_stream, parsed_line_storage);
        listing_pass(context, parsed_line_storage, listing);

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
