#include "listing.h"

#include "listing_line.h"
#include "options.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <sstream>
#include <vector>

Listing::Listing(std::ostream& output, const Options& options) : output(output), options(options) {}

void Listing::write_listing_header()
{
    time_t result = time(nullptr);
    std::string compile_time{asctime(localtime(&result))};

    output << "AS8 assembler for intel 8008, t.e.jones Version 1.0\n";
    output << "Options: listfile=" << options.generate_list_file << " debug=" << options.debug
           << " binaryout=" << options.generate_binary_file
           << " singlelist=" << options.single_byte_list << "\n";

    output << "octalnums=" << options.input_num_as_octal << " markascii=" << options.mark_8_ascii
           << "\n";
    output << "Infile=" << options.input_filename << "\n";
    output << "Assembly Performed: " << compile_time << "\n\n";
    if (options.single_byte_list)
    {
        output << "Line Addr.  DAT Source Line\n";
        output << "---- ------ --- ----------------------------------\n";
    }
    else
    {
        output << "Line Addr.  CodeBytes   Source Line\n";
        output << "---- ------ ----------- ----------------------------------\n";
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
    output << line.str() << "\n";
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
    output << line.str() << "\n";
}

void Listing::opcode_line_with_space(int line_number, int line_address,
                                     Opcode::OpcodeByteType opcode_byte,
                                     const std::string_view line_content)
{
    ListingLine line{line_number, line_address};
    line.add_byte(opcode_byte);
    line.add_line_content(line_content);
    output << line.str() << "\n";
}

void Listing::opcode_line_with_space_1_arg(int line_number, int line_address,
                                           Opcode::OpcodeByteType opcode_byte, int arg1,
                                           const std::string_view line_content)
{
    ListingLine line{line_number, line_address};
    line.add_byte(opcode_byte);
    line.add_byte(arg1);
    line.add_line_content(line_content);
    output << line.str() << "\n";
}

void Listing::opcode_line_with_space_2_arg(int line_number, int line_address,
                                           Opcode::OpcodeByteType opcode_byte, int arg1, int arg2,
                                           const std::string_view line_content)
{
    ListingLine line{line_number, line_address};
    line.add_byte(opcode_byte);
    line.add_byte(arg1);
    line.add_byte(arg2);
    line.add_line_content(line_content);
    output << line.str() << "\n";
}

void Listing::one_byte_of_data_with_address(int line_number, int line_address, int data,
                                            const std::string_view line_content) const
{
    ListingLine line{line_number, line_address};
    line.short_format();
    line.add_byte(data);
    line.add_line_content(line_content);
    output << line.str() << "\n";
}

void Listing::one_byte_of_data_continued(int line_address, int data) const
{
    ListingLine line;
    line.add_address(line_address);
    line.add_byte(data);
    output << line.str() << "\n";
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
            output << first_line.str() << "\n";

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
                output << next_line.str() << "\n";
            }
        }
    }
}
