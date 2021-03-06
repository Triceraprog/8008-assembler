#ifndef INC_8008_ASSEMBLER_LISTING_H
#define INC_8008_ASSEMBLER_LISTING_H

#include "opcodes/opcodes.h"

#include <vector>

class Options;

class Listing
{
public:
    Listing(std::ostream& output, const Options& options);
    void write_listing_header();
    void simple_line(uint32_t line_number, const std::string& line_content);
    void data(std::uint32_t line_number, int line_address, const std::string& line_content,
              const std::vector<int>& data_list);

    void reserved_data(uint32_t line_number, int line_address, const std::string& line_content);
    void one_byte_of_data_with_address(std::uint32_t line_number, int line_address, int data,
                                       std::string_view line_content) const;
    void one_byte_of_data_continued(int line_address, int data) const;
    void opcode_line_with_space(std::uint32_t line_number, int line_address,
                                Opcode::OpcodeByteType opcode_byte, std::string_view line_content);
    void opcode_line_with_space_1_arg(std::uint32_t line_number, int line_address,
                                      Opcode::OpcodeByteType opcode_byte, int arg1,
                                      std::string_view line_content);
    void opcode_line_with_space_2_arg(std::uint32_t line_number, int line_address,
                                      Opcode::OpcodeByteType opcode_byte, int arg1, int arg2,
                                      std::string_view line_content);
    bool short_format() const;

private:
    std::ostream& output;
    const Options& options;
};

#endif //INC_8008_ASSEMBLER_LISTING_H
