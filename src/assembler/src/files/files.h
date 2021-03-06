#ifndef INC_8008_ASSEMBLER_FILES_H
#define INC_8008_ASSEMBLER_FILES_H

#include "file_reader.h"

#include <exception>
#include <fstream>
#include <string>
#include <vector>

class Options;

class Files
{
public:
    explicit Files(const Options& options);
    ~Files();

    FileReader file_reader;
    std::fstream output_stream;
    std::fstream listing_stream;

private:
    void set_filenames(const Options& options);
    void open_files(const Options& options);

    std::string output_filename;
    std::string list_filename;
    std::vector<std::string> input_filenames;
};

class CannotOpenFile : std::exception
{
public:
    CannotOpenFile(const std::string& filename, const std::string& file_type_name);
    [[nodiscard]] const char* what() const noexcept override;

private:
    std::string reason;
};

#endif //INC_8008_ASSEMBLER_FILES_H
