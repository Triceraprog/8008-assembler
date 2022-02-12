#include "listing_line.h"

#include <iomanip>
#include <sstream>

ListingLine::ListingLine(int line_number)
{
    line.reserve(128);

    std::stringstream str;
    str << std::setw(4) << line_number << " ";

    line = std::move(str.str());
}

ListingLine::ListingLine(int line_number, int line_address)
{
    line.reserve(128);

    std::stringstream str;
    str << std::setw(4) << line_number << " ";

    str << std::setfill('0') << std::oct;
    str << std::setw(2) << ((line_address >> 8) & 0xFF) << "-";
    str << std::setw(3) << (line_address & 0xFF);

    line = std::move(str.str());
}

void ListingLine::add_byte(int byte)
{
    if (line.size() < 12)
    {
        add_padding(12);
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
    add_padding(24);
    line += line_content;
}

void ListingLine::add_padding(int column)
{
    if (line.size() < column)
    {
        auto spaces_to_add = column - line.size();
        line += std::string(spaces_to_add, ' ');
    }
}

std::string ListingLine::str() { return line; }
