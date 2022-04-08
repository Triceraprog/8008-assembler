#include "listing_pass.h"

#include "listing.h"
#include "options.h"
#include "parsed_line_storage.h"

#include <iostream>

void listing_pass(const Options& global_options, ParsedLineStorage& parsed_line_storage,
                  Listing& listing)
{
    if (!global_options.generate_list_file)
    {
        return;
    }

    if (global_options.verbose || global_options.debug)
    {
        std::cout << "Pass number Three:  Re-read and assemble codes\n";
    }

    listing.write_listing_header();

    for (auto& parsed_line : parsed_line_storage)
    {
        const auto& input_line = parsed_line.line;
        const auto line_number = parsed_line.line_number;
        int line_address = parsed_line.line_address;

        const auto& instruction = parsed_line.instruction;
        instruction.listing_pass(listing, input_line, line_number, line_address);
    }
}
