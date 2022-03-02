#include "listing_line.h"

#include <iomanip>
#include <sstream>

ListingLine::ListingLine() { line.reserve(128); }

ListingLine::ListingLine(int line_number) : ListingLine()
{
    std::stringstream str;
    str << std::setw(4) << line_number << " ";

    line = std::move(str.str());
}

ListingLine::ListingLine(int line_number, int line_address) : ListingLine()
{
    std::stringstream str;
    str << std::setw(4) << line_number << " ";
    line = std::move(str.str());

    add_address(line_address);
}

void ListingLine::short_format() { column_for_padding = 16; }

void ListingLine::add_address(int line_address)
{
    padding(5);

    std::stringstream str;
    str << std::setfill('0') << std::oct;
    str << std::setw(2) << ((line_address >> 8) & 0xFF) << "-";
    str << std::setw(3) << (line_address & 0xFF);

    line += str.str();
}

void ListingLine::add_byte(int byte)
{
    if (line.size() < 12)
    {
        padding(12);
    }
    else
    {
        line += " ";
    }

    std::stringstream str;
    str << std::setfill('0') << std::setw(3) << std::oct << byte;

    line += str.str();
}

void ListingLine::add_line_content(std::string_view line_content)
{
    padding(column_for_padding);
    line += line_content;
}

void ListingLine::padding(int column)
{
    if (line.size() < column)
    {
        auto spaces_to_add = column - line.size();
        line += std::string(spaces_to_add, ' ');
    }
}

std::string ListingLine::str() const { return line; }
