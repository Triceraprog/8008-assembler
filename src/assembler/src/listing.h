#ifndef INC_8008_ASSEMBLER_LISTING_H
#define INC_8008_ASSEMBLER_LISTING_H

#include "opcodes.h"

#include <vector>

class Options;

class Listing
{
public:
    Listing(std::ostream& output, const Options& options);
    void write_listing_header();
    void simple_line(int line_number, const std::string& line_content, bool short_format);
    void data(int line_number, int line_address, const std::string& line_content,
              const std::vector<int>& data_list);

    void reserved_data(int line_number, int line_address, const std::string& line_content,
                       bool short_format);
    void one_byte_of_data_with_address(int line_number, int line_address, int data,
                                       std::string_view line_content) const;
    void one_byte_of_data_continued(int line_address, int data) const;
    void opcode_line_with_space(int line_number, int line_address,
                                Opcode::OpcodeByteType opcode_byte, std::string_view line_content);
    void opcode_line_with_space_1_arg(int line_number, int line_address,
                                      Opcode::OpcodeByteType opcode_byte, int arg1,
                                      std::string_view line_content);
    void opcode_line_with_space_2_arg(int line_number, int line_address,
                                      Opcode::OpcodeByteType opcode_byte, int arg1, int arg2,
                                      std::string_view line_content);

private:
    std::ostream& output;
    const Options& options;
};

#endif //INC_8008_ASSEMBLER_LISTING_H
