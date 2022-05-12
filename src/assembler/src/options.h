#ifndef INC_8008_ASSEMBLER_OPTIONS_H
#define INC_8008_ASSEMBLER_OPTIONS_H

#include <exception>
#include <string>
#include <vector>

class InvalidCommandLine : std::exception
{
    [[nodiscard]] const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override;
};

class Options
{
public:
    Options() = default;
    void parse(int argc, const char** argv);

public:
    bool verbose = false;
    bool generate_list_file = true;
    bool debug = false;
    bool single_byte_list = false;
    bool generate_binary_file = false;
    bool input_num_as_octal = false;
    bool mark_8_ascii = false;
    bool new_syntax = false;
    bool output_name = false;
    size_t data_per_line_limit = 128;

    std::vector<std::string> input_filenames;
    std::string output_filename_base;

private:
    std::size_t parse_command_line(int argc, const char** argv);
    static void display_help(const char** argv);
    void adjust_filenames();
};

#endif //INC_8008_ASSEMBLER_OPTIONS_H
