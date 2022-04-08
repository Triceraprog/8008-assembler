#include "assembler/src/errors.h"
#include "assembler/src/files/files.h"
#include "assembler/src/first_pass.h"
#include "assembler/src/listing.h"
#include "assembler/src/listing_pass.h"
#include "assembler/src/options.h"
#include "assembler/src/parsed_line_storage.h"
#include "assembler/src/second_pass.h"
#include "context_stack.h"

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
        Listing listing(files.listing_stream, global_options);
        ParsedLineStorage parsed_line_storage;

        ContextStack context_stack(global_options);

        first_pass(context_stack, files.file_reader, parsed_line_storage);
        second_pass(global_options, files.output_stream, parsed_line_storage);
        listing_pass(global_options, parsed_line_storage, listing);

        /* write symbol table to listfile */
        if (global_options.generate_list_file)
        {
            context_stack.get_current_context()->list_symbols(files.listing_stream);
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
