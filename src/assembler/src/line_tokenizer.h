#ifndef INC_8008_ASSEMBLER_LINE_TOKENIZER_H
#define INC_8008_ASSEMBLER_LINE_TOKENIZER_H

#include <deque>
#include <string>

class Options;

class LineTokenizer
{
public:
    explicit LineTokenizer(const std::string& line);
    void debug_print() const;

    std::string label;
    std::string opcode;
    std::string arg1;
    std::string arg2;

    std::uint32_t arg_count{};
    bool warning_on_label{false};

private:
    static std::string consume_parsed(std::deque<std::string>& parsed);
    void adjust_label();
};

LineTokenizer parse_line(const Options& options, const std::string& line, int line_count);

#endif //INC_8008_ASSEMBLER_LINE_TOKENIZER_H
