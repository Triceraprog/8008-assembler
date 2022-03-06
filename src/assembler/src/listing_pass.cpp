#include "listing_pass.h"

#include "context.h"
#include "files.h"
#include "listing.h"
#include "options.h"
#include "parsed_line.h"

#include <iostream>

void listing_pass(const Context& context, Files& files, const std::vector<ParsedLine>& parsed_lines,
                  Listing& listing)
{
    const auto& options = context.options;
    if (!options.generate_list_file)
    {
        return;
    }

    if (options.verbose || options.debug)
    {
        std::cout << "Pass number Three:  Re-read and assemble codes\n";
    }

    listing.write_listing_header();

    for (auto& parsed_line : parsed_lines)
    {
        const auto& input_line = parsed_line.line;
        const int line_number = parsed_line.line_number;
        int line_address = parsed_line.line_address;

        const auto& instruction = parsed_line.instruction;
        instruction.listing_pass(listing, input_line, line_number, line_address,
                                 context.options.single_byte_list);
    }
}
