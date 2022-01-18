#ifndef INC_8008_ASSEMBLER_OPTIONS_H
#define INC_8008_ASSEMBLER_OPTIONS_H

#include <exception>
#include <string>

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
    int verbose = 0;
    int generate_list_file = 1;
    int debug = 0;
    int single_byte_list = 0;
    int generate_binary_file = 0;
    int input_num_as_octal = 0;
    int mark_8_ascii = 0;

    std::string input_filename;

private:
    std::size_t parse_command_line(int argc, const char** argv);
    static void display_help(const char** argv);
};

#endif //INC_8008_ASSEMBLER_OPTIONS_H
