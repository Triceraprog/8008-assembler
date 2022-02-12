#ifndef INC_8008_ASSEMBLER_LISTING_H
#define INC_8008_ASSEMBLER_LISTING_H

#include "options.h"

#include <cstdio>
#include <vector>

struct Opcode;

class Listing
{
public:
    Listing(FILE* output, const Options& options);
    void write_listing_header();
    void simple_line(int line_number, const std::string& line_content);
    void data(int line_number, int line_address, const std::string& line_content,
              const std::vector<int>& data_list);

    void reserved_data(int line_number, int line_address, const std::string& line_content);
    void one_byte_of_data_with_address(int line_number, int line_address, int data,
                                       const std::string& line_content) const;
    void one_byte_of_data_continued(int line_address, int data) const;
    void opcode_line_with_space(int line_number, int line_address, const Opcode& opcode,
                                const std::string& line_content);
    void opcode_line_with_space_1_arg(int line_number, int line_address, const Opcode& opcode,
                                      int arg1, const std::string& line_content);
    void opcode_line_with_space_2_arg(int line_number, int line_address, const Opcode& opcode,
                                      int arg1, int arg2, const std::string& line_content);

private:
    FILE* output;
    const Options& options;
    char single_space_pad[9]{}; /* this is some extra padding if we use single space list file */
    void write_line_number(int line_number) const;
    void write_address(int address) const;
    void write_line_preamble(int line_number, int line_address) const;
};

#endif //INC_8008_ASSEMBLER_LISTING_H
