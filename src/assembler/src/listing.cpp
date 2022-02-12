#include "listing.h"

#include "listing_line.h"
#include "opcodes.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <sstream>
#include <vector>

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

void Listing::write_line_number(int line_number) const { fprintf(output, "%4d ", line_number); }
void Listing::write_address(int address) const
{
    fprintf(output, "%02o-%03o", ((address >> 8) & 0xFF), (address & 0xFF));
}

void Listing::simple_line(int line_number, const std::string& line_content)
{
    write_line_number(line_number);
    fprintf(output, "           %s%s\n", single_space_pad, line_content.c_str());
}

void Listing::write_line_preamble(int line_number, int line_address) const
{
    write_line_number(line_number);
    write_address(line_address);
    fprintf(output, " ");
}

void Listing::reserved_data(int line_number, int line_address, const std::string& line_content)
{
    write_line_preamble(line_number, line_address);
    fprintf(output, "    %s%s\n", single_space_pad, line_content.c_str());
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
    write_line_preamble(line_number, line_address);
    fprintf(output, "%03o %03o     %s\n", opcode.code, arg1, line_content.c_str());
}

void Listing::opcode_line_with_space_2_arg(int line_number, int line_address, const Opcode& opcode,
                                           int arg1, int arg2, const std::string& line_content)
{
    write_line_preamble(line_number, line_address);
    fprintf(output, "%03o %03o %03o %s\n", opcode.code, arg1, arg2, line_content.c_str());
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
