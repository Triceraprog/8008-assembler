#ifndef INC_8008_ASSEMBLER_LISTING_LINE_H
#define INC_8008_ASSEMBLER_LISTING_LINE_H

#include <string>

class ListingLine
{
public:
    ListingLine();
    ListingLine(int line_number);

    ListingLine(int line_number, int line_address);
    void add_line_content(std::string_view line_content);
    void add_byte(int byte);
    void add_address(int line_address);

    std::string str();

private:
    std::string line;

    void padding(int column);
};

#endif //INC_8008_ASSEMBLER_LISTING_LINE_H
