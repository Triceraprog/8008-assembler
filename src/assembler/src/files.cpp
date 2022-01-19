#include "files.h"

#include "options.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>

Files::Files(const Options& options)
{
    set_output_filenames(options);
    open_files(options);
}

void Files::set_output_filenames(const Options& options)
{
    const auto* output_filename_extension = options.generate_binary_file ? ".bin" : ".hex";
    outfilename = options.input_filename_base + output_filename_extension;
    listfilename = options.input_filename_base + ".lst";
    input_filename = options.input_filename;

    if (options.debug)
    {
        std::cout << "filebase=" << options.input_filename_base << " ";
        std::cout << "infile=" << input_filename << " ";
        std::cout << "outfile=" << outfilename << " ";
        std::cout << "listfile=" << listfilename << "\n";
    }
}

void Files::open_files(const Options& options)
{
    if ((ifp = fopen(input_filename.c_str(), "rt")) == nullptr)
    {
        fprintf(stderr, "Can't open %s as input file\n", options.input_filename.c_str());
        exit(-1);
    }
    if (options.generate_binary_file)
    {
        if ((ofp = fopen(outfilename.c_str(), "wb")) == nullptr)
        {
            fprintf(stderr, "Can't open %s as output file\n", outfilename.c_str());
            exit(-1);
        }
    }
    else
    {
        if ((ofp = fopen(outfilename.c_str(), "wt")) == nullptr)
        {
            fprintf(stderr, "Can't open %s as output file\n", outfilename.c_str());
            exit(-1);
        }
    }
    if (options.generate_list_file)
    {
        if ((lfp = fopen(listfilename.c_str(), "wt")) == nullptr)
        {
            fprintf(stderr, "Can't open %s as input file\n", listfilename.c_str());
            exit(-1);
        }
    }
    if (options.debug)
    {
        printf("All files were opened.\n");
    }
}
