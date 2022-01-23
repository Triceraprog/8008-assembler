#include "files.h"

#include "options.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

Files::Files(const Options& options)
{
    set_output_filenames(options);
    open_files(options);
}

Files::~Files()
{
    if (input_stream.is_open())
    {
        input_stream.close();
    }
}

void Files::set_output_filenames(const Options& options)
{
    const auto* output_filename_extension = options.generate_binary_file ? ".bin" : ".hex";
    output_filename = options.input_filename_base + output_filename_extension;
    list_filename = options.input_filename_base + ".lst";
    input_filename = options.input_filename;

    if (options.debug)
    {
        std::cout << "filebase=" << options.input_filename_base << " ";
        std::cout << "infile=" << input_filename << " ";
        std::cout << "outfile=" << output_filename << " ";
        std::cout << "listfile=" << list_filename << "\n";
    }
}

void Files::open_files(const Options& options)
{
    input_stream.open(input_filename.c_str());

    if (input_stream.fail())
    {
        std::cerr << "Can't open " << options.input_filename << " as input file\n";
        exit(-1);
    }

    if (options.generate_binary_file)
    {
        output_stream.open(output_filename.c_str(), std::ios::binary | std::ios::out);
        if (output_stream.fail())
        {
            std::cerr << "Can't open " << options.input_filename << " as binary output file\n";
            exit(-1);
        }
    }
    else
    {
        output_stream.open(output_filename.c_str(), std::ios::out);
        if (output_stream.fail())
        {
            std::cerr << "Can't open " << options.input_filename << " as hex output file\n";
            exit(-1);
        }
    }
    
    if (options.generate_list_file)
    {
        if ((lfp = fopen(list_filename.c_str(), "wt")) == nullptr)
        {
            fprintf(stderr, "Can't open %s as input file\n", list_filename.c_str());
            exit(-1);
        }
    }
    if (options.debug)
    {
        printf("All files were opened.\n");
    }
}
