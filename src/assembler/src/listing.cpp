#include "listing.h"

#include <cstring>

Listing::Listing(FILE* output, const Options& options) : output(output), options(options)
{
    if (options.single_byte_list)
    {
        single_space_pad[0] = 0;
    }

    else
    {
        strcpy(single_space_pad, "        ");
    }
}

void Listing::write_listing_header()
{
    time_t result = time(nullptr);
    std::string compile_time{asctime(localtime(&result))};

    fprintf(output, "AS8 assembler for intel 8008, t.e.jones Version 1.0\n");
    fprintf(output, "Options: listfile=%d debug=%d ", options.generate_list_file, options.debug);
    fprintf(output, "binaryout=%d singlelist=%d\n", options.generate_binary_file,
            options.single_byte_list);
    fprintf(output, "octalnums=%d markascii=%d\n", options.input_num_as_octal,
            options.mark_8_ascii);
    fprintf(output, "Infile=%s\n", options.input_filename.c_str());
    fprintf(output, "Assembly Performed: %s\n\n", compile_time.c_str());
    if (options.single_byte_list)
    {
        fprintf(output, "Line Addr.  DAT Source Line\n");
        fprintf(output, "---- ------ --- ----------------------------------\n");
    }
    else
    {
        fprintf(output, "Line Addr.  CodeBytes   Source Line\n");
        fprintf(output, "---- ------ ----------- ----------------------------------\n");
    }
}

void Listing::simple_line(int line_number, const std::string& line_content)
{
    fprintf(output, "%4d            %s%s\n", line_number, single_space_pad, line_content.c_str());
}
