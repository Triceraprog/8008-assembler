#include "second_pass.h"
#include "byte_writer.h"
#include "context.h"
#include "errors.h"
#include "options.h"
#include "parsed_line_storage.h"
#include "symbol_table.h"

#include <cstdio>
#include <iostream>

void second_pass(const Options& global_options, std::ostream& output_stream,
                 ParsedLineStorage& parsed_line_storage)
{
    /* Symbols are defined. Second pass. */
    if (global_options.verbose || global_options.debug)
    {
        std::cout << "Pass number Two:  Re-read and assemble codes\n";
    }

    ByteWriter writer(output_stream,
                      global_options.generate_binary_file ? ByteWriter::BINARY : ByteWriter::HEX);

    for (auto& parsed_line : parsed_line_storage)
    {
        const auto& input_line = parsed_line.line;
        const auto line_number = parsed_line.line_number;
        int line_address = parsed_line.line_address;

        try
        {
            if (global_options.verbose || global_options.debug)
            {
                printf("     0x%X \"%s\"\n", line_address, input_line.c_str());
            }

            const auto& instruction = parsed_line.instruction;
            const auto& associated_context = parsed_line.context;
            instruction.second_pass(*associated_context, writer, line_address);
        }
        catch (const std::exception& ex)
        {
            throw ParsingException(ex, line_number, *parsed_line.name_tag, input_line);
        }
    }
    writer.write_end();
}
