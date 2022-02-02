#ifndef INC_8008_ASSEMBLER_FILES_H
#define INC_8008_ASSEMBLER_FILES_H

#include <exception>
#include <fstream>
#include <string>

class CannotOpenFile : std::exception
{
public:
    CannotOpenFile(const std::string& filename, const std::string& file_type_name);
    [[nodiscard]] const char* what() const noexcept override;

private:
    std::string reason;
};

class Options;

class Files
{
public:
    explicit Files(const Options& options);
    ~Files();

    FILE* lfp{};
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
