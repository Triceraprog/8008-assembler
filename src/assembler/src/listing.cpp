#include "listing.h"

#include "listing_line.h"
#include "opcodes.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <sstream>
#include <vector>

Listing::Listing(FILE* output, const Options& options) : output(output), options(options) {}

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

void Listing::simple_line(int line_number, const std::string& line_content, const bool short_format)
{
    ListingLine line{line_number};
    if (short_format)
    {
        line.short_format();
    }
    line.add_line_content(line_content);
    fprintf(output, "%s\n", line.str().c_str());
}

void Listing::reserved_data(int line_number, int line_address, const std::string& line_content,
                            const bool short_format)
{
    ListingLine line{line_number, line_address};
    if (short_format)
    {
        line.short_format();
    }
    line.add_line_content(line_content);
    fprintf(output, "%s\n", line.str().c_str());
}

void Listing::opcode_line_with_space(int line_number, int line_address, const Opcode& opcode,
                                     const std::string& line_content)
{
    ListingLine line{line_number, line_address};
    line.add_byte(opcode.code);
    line.add_line_content(line_content);
    fprintf(output, "%s\n", line.str().c_str());
}

void Listing::opcode_line_with_space_1_arg(int line_number, int line_address, const Opcode& opcode,
                                           int arg1, const std::string& line_content)
{
    ListingLine line{line_number, line_address};
    line.add_byte(opcode.code);
    line.add_byte(arg1);
    line.add_line_content(line_content);
    fprintf(output, "%s\n", line.str().c_str());
}

void Listing::opcode_line_with_space_2_arg(int line_number, int line_address, const Opcode& opcode,
                                           int arg1, int arg2, const std::string& line_content)
{
    ListingLine line{line_number, line_address};
    line.add_byte(opcode.code);
    line.add_byte(arg1);
    line.add_byte(arg2);
    line.add_line_content(line_content);
    fprintf(output, "%s\n", line.str().c_str());
}

void Listing::one_byte_of_data_with_address(int line_number, int line_address, int data,
                                            const std::string& line_content) const
{
    ListingLine line{line_number, line_address};
    line.short_format();
    line.add_byte(data);
    line.add_line_content(line_content);
    fprintf(output, "%s\n", line.str().c_str());
}

void Listing::one_byte_of_data_continued(int line_address, int data) const
{
    ListingLine line;
    line.add_address(line_address);
    line.add_byte(data);
    fprintf(output, "%s\n", line.str().c_str());
}

void Listing::data(int line_number, int line_address, const std::string& line_content,
                   const std::vector<int>& data_list)
{
    if (options.single_byte_list)
    {
        one_byte_of_data_with_address(line_number, line_address, data_list[0], line_content);

        for (int data : data_list | std::views::drop(1))
        {
            line_address += 1;
            one_byte_of_data_continued(line_address, data);
        }
    }
    else
    {
        {
            int index = 0;

            // First line
            ListingLine first_line{line_number, line_address};
            for (int data : data_list | std::views::take(3))
            {
                first_line.add_byte(data);
                index += 1;
                line_address += 1;
            }
            first_line.add_line_content(line_content);
            fprintf(output, "%s\n", first_line.str().c_str());

            // Next lines
            while (data_list.size() > index)
            {
                ListingLine next_line;
                next_line.add_address(line_address);

                for (int data : data_list | std::views::drop(index) | std::views::take(3))
                {
                    next_line.add_byte(data);
                    index += 1;
                    line_address += 1;
                }
                fprintf(output, "%s\n", next_line.str().c_str());
            }
        }
    }
}
