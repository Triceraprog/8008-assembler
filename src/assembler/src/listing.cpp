#include "listing.h"

#include "opcodes.h"

#include <cstring>
#include <ranges>
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

void Listing::simple_line(int line_number, const std::string& line_content)
{
    fprintf(output, "%4d            %s%s\n", line_number, single_space_pad, line_content.c_str());
}

void Listing::reserved_data(int line_number, int line_address, const std::string& line_content)
{
    fprintf(output, "%4d %02o-%03o     %s%s\n", line_number, ((line_address >> 8) & 0xFF),
            (line_address & 0xFF), single_space_pad, line_content.c_str());
}

void Listing::opcode_line_with_space(int line_number, int line_address, const Opcode& opcode,
                                     const std::string& line_content)
{
    fprintf(output, "%4d %02o-%03o %03o %s%s\n", line_number, ((line_address >> 8) & 0xFF),
            (line_address & 0xFF), opcode.code, single_space_pad, line_content.c_str());
}

void Listing::data(int line_number, int line_address, const std::string& line_content,
                   const std::vector<int>& data_list)
{
    if (options.single_byte_list)
    {
        int line_address_h = (line_address >> 8) & 0xFF;
        int line_address_l = line_address & 0xFF;

        fprintf(output, "%4d %02o-%03o %03o %s\n", line_number, line_address_h, line_address_l,
                data_list[0], line_content.c_str());
        for (int data : data_list | std::views::drop(1))
        {
            line_address += 1;
            fprintf(output, "%4d %02o-%03o %03o\n", line_number, (((line_address) >> 8) & 0xFF),
                    ((line_address) &0xFF), data);
        }
    }
    else
    {
        int line_address_h = (line_address >> 8) & 0xFF;
        int line_address_l = line_address & 0xFF;

        fprintf(output, "%4d %02o-%03o ", line_number, line_address_h, line_address_l);
        if (data_list.size() == 1)
            fprintf(output, "%03o          %s\n", data_list[0], line_content.c_str());
        else if (data_list.size() == 2)
            fprintf(output, "%03o %03o      %s\n", data_list[0], data_list[1],
                    line_content.c_str());
        else if (data_list.size() > 2)
        {
            fprintf(output, "%03o %03o %03o  %s\n", data_list[0], data_list[1], data_list[2],
                    line_content.c_str());
            int index = 3;
            line_address += 3;
            while (data_list.size() > index)
            {
                line_address_h = (line_address >> 8) & 0xFF;
                line_address_l = line_address & 0xFF;

                /*	    fprintf(output,"            "); */
                fprintf(output, "     %02o-%03o ", line_address_h, line_address_l);
                if (data_list.size() > 2 + index)
                {
                    fprintf(output, "%03o %03o %03o\n", data_list[index], data_list[index + 1],
                            data_list[index + 2]);
                    index += 3;
                    line_address += 3;
                }
                else
                {
                    for (int data : data_list | std::views::drop(index))
                    {
                        fprintf(output, "%03o ", data);
                        index += 1;
                    }
                    fprintf(output, "\n");
                }
            }
        }
    }
}
