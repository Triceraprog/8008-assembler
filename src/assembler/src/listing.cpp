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

void Listing::data(int line_number, int line_address, const std::string& line_content,
                   int* data_list, int data_list_length)
{
    if (options.single_byte_list)
    {
        int line_address_h = (line_address >> 8) & 0xFF;
        int line_address_l = line_address & 0xFF;

        fprintf(output, "%4d %02o-%03o %03o %s\n", line_number, line_address_h, line_address_l,
                data_list[0], line_content.c_str());
        for (int i = 1; i < data_list_length; i++)
        {
            fprintf(output, "%4d %02o-%03o %03o\n", line_number, (((line_address + i) >> 8) & 0xFF),
                    ((line_address + i) & 0xFF), data_list[i]);
        }
    }
    else
    {
        int line_address_h = (line_address >> 8) & 0xFF;
        int line_address_l = line_address & 0xFF;

        fprintf(output, "%4d %02o-%03o ", line_number, line_address_h, line_address_l);
        if (data_list_length == 1)
            fprintf(output, "%03o          %s\n", data_list[0], line_content.c_str());
        else if (data_list_length == 2)
            fprintf(output, "%03o %03o      %s\n", data_list[0], data_list[1],
                    line_content.c_str());
        else if (data_list_length > 2)
        {
            fprintf(output, "%03o %03o %03o  %s\n", data_list[0], data_list[1], data_list[2],
                    line_content.c_str());
            int* ptr = data_list + 3;
            data_list_length -= 3;
            line_address += 3;
            while (data_list_length > 0)
            {
                line_address_h = (line_address >> 8) & 0xFF;
                line_address_l = line_address & 0xFF;

                /*	    fprintf(output,"            "); */
                fprintf(output, "     %02o-%03o ", line_address_h, line_address_l);
                if (data_list_length > 2)
                {
                    fprintf(output, "%03o %03o %03o\n", ptr[0], ptr[1], ptr[2]);
                    ptr += 3;
                    data_list_length -= 3;
                    line_address += 3;
                }
                else
                {
                    for (int i = 0; i < data_list_length; i++)
                    {
                        fprintf(output, "%03o ", ptr[0]);
                        ptr++;
                    }
                    data_list_length = 0;
                    fprintf(output, "\n");
                }
            }
        }
    }
}
