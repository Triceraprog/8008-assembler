#include "files.h"

#include "options.h"
#include "file_utility.h"

#include <iostream>

Files::Files(const Options& options)
{
    set_filenames(options);
    open_files(options);
}

Files::~Files() = default;

void Files::set_filenames(const Options& options)
{
    const auto* output_filename_extension = options.generate_binary_file ? ".bin" : ".hex";
    output_filename = options.output_filename_base + output_filename_extension;
    list_filename = options.output_filename_base + ".lst";
    std::ranges::copy(options.input_filenames, std::back_inserter(input_filenames));

    if (options.debug)
    {
        std::cout << "filebase=" << options.output_filename_base << " ";
        std::cout << "infile=" << input_filenames.front() << " ";
        std::cout << "outfile=" << output_filename << " ";
        std::cout << "listfile=" << list_filename << "\n";
    }
}

void Files::open_files(const Options& options)
{
    for (const auto& input_filename : input_filenames)
    {
        Utility::append_file_by_name(file_reader, input_filename);
    }

    if (options.generate_binary_file)
    {
        output_stream.open(output_filename.c_str(), std::ios::binary | std::ios::out);
        if (output_stream.fail())
        {
            throw CannotOpenFile(output_filename, "binary output file");
        }
    }
    else
    {
        output_stream.open(output_filename.c_str(), std::ios::out);
        if (output_stream.fail())
        {
            throw CannotOpenFile(output_filename, "hex output file");
        }
    }

    if (options.generate_list_file)
    {
        listing_stream.open(list_filename.c_str(), std::ios::out);
        if (listing_stream.fail())
        {
            throw CannotOpenFile(list_filename, "output list file");
        }
    }
    if (options.debug)
    {
        printf("All files were opened.\n");
    }
}

CannotOpenFile::CannotOpenFile(const std::string& filename, const std::string& file_type_name)
    : reason{"Can't open " + filename + " as " + file_type_name}
{}
const char* CannotOpenFile::what() const noexcept { return reason.c_str(); }
