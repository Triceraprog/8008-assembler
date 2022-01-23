#ifndef INC_8008_ASSEMBLER_FILES_H
#define INC_8008_ASSEMBLER_FILES_H

#include <string>
#include <fstream>

class Options;

class Files
{
public:
    explicit Files(const Options& options);
    ~Files();

    FILE *lfp{};
    std::fstream input_stream;
    std::fstream output_stream;

private:
    void set_output_filenames(const Options& options);
    void open_files(const Options& options);

    std::string output_filename;
    std::string list_filename;
    std::string input_filename;
};

#endif //INC_8008_ASSEMBLER_FILES_H
