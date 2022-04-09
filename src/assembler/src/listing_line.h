#ifndef INC_8008_ASSEMBLER_LISTING_LINE_H
#define INC_8008_ASSEMBLER_LISTING_LINE_H

#include <string>

class ListingLine
{
public:
    ListingLine();
    explicit ListingLine(std::uint32_t line_number);
    ListingLine(uint32_t line_number, int line_address);

    void add_line_content(std::string_view line_content);
    void add_byte(int byte);
    void add_address(int line_address);

    [[nodiscard]] std::string str() const;

    void short_format();

private:
    std::string line;
    int column_for_padding = 24;

    void padding(int column);
};

#endif //INC_8008_ASSEMBLER_LISTING_LINE_H
