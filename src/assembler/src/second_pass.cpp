#include "second_pass.h"
#include "byte_writer.h"
#include "context.h"
#include "errors.h"
#include "evaluator.h"
#include "files.h"
#include "listing.h"
#include "options.h"
#include "parsed_line.h"
#include "symbol_table.h"

#include <cstdio>
#include <iostream>

void second_pass(const Context& context, const Options& options, const SymbolTable& symbol_table,
                 Files& files, const std::vector<ParsedLine>& parsed_lines, Listing& listing)
{
    /* Symbols are defined. Second pass. */
    if (options.verbose || options.debug)
    {
        std::cout << "Pass number Two:  Re-read and assemble codes\n";
    }

    listing.write_listing_header();

    ByteWriter writer(files.output_stream,
                      options.generate_binary_file ? ByteWriter::BINARY : ByteWriter::HEX);

    for (auto& parsed_line : parsed_lines)
    {
        const auto& input_line = parsed_line.line;
        const int line_number = parsed_line.line_number;
        int line_address = parsed_line.line_address;

        try
        {
            if (options.verbose || options.debug)
            {
                printf("     0x%X \"%s\"\n", line_address, input_line.c_str());
            }

            const auto& instruction = parsed_line.instruction;
            instruction.second_pass(context, listing, writer, input_line, line_number,
                                    line_address);
        }
        catch (const CannotFindSymbol& ex)
        {
            throw ParsingException(ex, line_number, input_line);
        }
    }

    if (options.generate_list_file)
    {
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

    writer.write_end();
}
