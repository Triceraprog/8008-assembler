#include "options.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>

const char* InvalidCommandLine::what() const noexcept { return "Invalid Command Line"; }

void Options::parse(int argc, const char** argv)
{
    argc = parse_command_line(argc, argv);
    if (argc != 1)
    {
        display_help(argv);
        throw InvalidCommandLine();
    }
}

std::size_t Options::parse_command_line(int argc, const char** argv)
{
    std::vector<std::string_view> argv_vector{argv, argv + argc};
    std::vector<std::string_view> left_arguments{};

    for (auto& arg : argv_vector | std::ranges::views::drop(1))
    {
        if (arg == "-v")
        {
            verbose = 1;
        }
        else if (arg == "-nl")
        {
            generate_list_file = 0;
        }
        else if (arg == "-d")
        {
            debug = 1;
        }
        else if (arg == "-bin")
        {
            generate_binary_file = 1;
        }
        else if (arg == "-octal")
        {
            input_num_as_octal = 1;
        }
        else if (arg == "-single")
        {
            single_byte_list = 1;
        }
        else if (arg == "-markascii")
        {
            mark_8_ascii = 1;
        }
        else if (arg[0] == '-')
        {
            std::cerr << "unknown option " << arg << "\n";
            std::cerr << "    type " << argv_vector[0] << " for usage" << std::endl;
        }
        else
        {
            left_arguments.push_back(arg);
        }
    }

    if (!left_arguments.empty())
    {
        input_filename = left_arguments[0];
    }

    return left_arguments.size();
}

void Options::display_help(const char** argv)
{
    fprintf(stderr, "Usage: %s [options] infile\n", argv[0]);
    fprintf(stderr, "    where <infile> is assembly code file, extension defaults to .asm\n");
    fprintf(stderr, "    and options include...\n");
    fprintf(stderr, "    -v        verbose output\n");
    fprintf(stderr, "    -nl       no list (default is to make .lst file.)\n");
    fprintf(stderr, "    -d        debug assembler (extra output)\n");
    fprintf(stderr, "    -bin      makes output binary ROM file, otherwise intel hex\n");
    fprintf(stderr, "    -octal    makes unidentified 3-digit numbers octal (default decimal)\n");
    fprintf(stderr, "    -single   makes .lst file single byte per line, otherwise 3/line.\n");
    fprintf(stderr, "    -markascii makes highest bit in ascii bytes a one (mark).\n");
}
