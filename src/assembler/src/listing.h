#ifndef INC_8008_ASSEMBLER_LISTING_H
#define INC_8008_ASSEMBLER_LISTING_H

#include "options.h"

#include <cstdio>
#include <vector>

class Listing
{
public:
    Listing(FILE* output, const Options& options);
    void write_listing_header();
    void simple_line(int line_number, const std::string& line_content);
    void data(int line_number, int line_address, const std::string& line_content,
              const std::vector<int>& data_list);

private:
    FILE* output;
    const Options& options;
    char single_space_pad[9]{}; /* this is some extra padding if we use single space list file */
};

#endif //INC_8008_ASSEMBLER_LISTING_H
