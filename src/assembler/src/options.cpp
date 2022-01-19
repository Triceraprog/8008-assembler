#include "options.h"

#include <algorithm>
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

    adjust_filenames();
}

std::size_t Options::parse_command_line(int argc, const char** argv)
{
    std::vector<std::string_view> argv_vector{argv, argv + argc};
    std::vector<std::string_view> left_arguments{};

    using option_selector = std::tuple<std::string_view, bool*, bool>;
    std::vector<option_selector> options = {{"-v", &verbose, true},
                                            {"-nl", &generate_list_file, false},
                                            {"-d", &debug, true},
                                            {"-bin", &generate_binary_file, true},
                                            {"-octal", &input_num_as_octal, true},
                                            {"-single", &single_byte_list, true},
                                            {"-markascii", &mark_8_ascii, true}};

    for (auto& arg : argv_vector | std::ranges::views::drop(1))
    {
        if (arg[0] == '-')
        {
            auto found_option = std::ranges::find_if(
                    options, [&arg](const auto& option) { return arg == std::get<0>(option); });

            if (found_option != options.end())
            {
                *std::get<1>(*found_option) = std::get<2>(*found_option);
            }
            else
            {
                std::cerr << "unknown option " << arg << "\n";
                std::cerr << "    type " << argv_vector[0] << " for usage" << std::endl;
            }
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

void Options::adjust_filenames()
{
    if (auto dot_index = input_filename.find('.'); dot_index != std::string::npos)
    {
        input_filename_base = input_filename.substr(0, dot_index);
    }
    else
    {
        input_filename_base = input_filename;
        input_filename += ".asm";
    }
}