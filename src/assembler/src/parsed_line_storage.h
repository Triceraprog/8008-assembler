#ifndef INC_8008_ASSEMBLER_PARSED_LINE_STORAGE_H
#define INC_8008_ASSEMBLER_PARSED_LINE_STORAGE_H

#include "context.h"
#include "parsed_line.h"

#include <string_view>
#include <vector>

class Context;
class FileReader;

class ParsedLineStorage
{
public:
    void append_line(const Context& context, FileReader& file_reader, std::string_view input_line,
                     std::size_t line_number, int address);

    [[nodiscard]] const ParsedLine& latest_line() const;

    using Iterator = std::vector<ParsedLine>::iterator;
    Iterator begin();
    Iterator end();

private:
    std::vector<ParsedLine> parsed_lines;
};

#endif //INC_8008_ASSEMBLER_PARSED_LINE_STORAGE_H
