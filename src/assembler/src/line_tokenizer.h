#ifndef INC_8008_ASSEMBLER_LINE_TOKENIZER_H
#define INC_8008_ASSEMBLER_LINE_TOKENIZER_H

#include <deque>
#include <string>
#include <vector>

class Options;

class LineTokenizer
{
public:
    explicit LineTokenizer(std::string_view line);

    std::string label;
    std::string opcode;
    std::vector<std::string> arguments;
    std::string comment;

    bool warning_on_label{false};

private:
    void adjust_label();
};

LineTokenizer parse_line(const Options& options, std::string_view line, std::size_t line_count);

#endif //INC_8008_ASSEMBLER_LINE_TOKENIZER_H
