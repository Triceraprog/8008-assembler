#ifndef INC_8008_ASSEMBLER_FILE_UTILITY_H
#define INC_8008_ASSEMBLER_FILE_UTILITY_H

#include <string>

class FileReader;

namespace Utility
{
    void append_file_by_name(FileReader& file_reader, const std::string& filename);
    void insert_file_by_name(FileReader& file_reader, const std::string& filename);
}

#endif //INC_8008_ASSEMBLER_FILE_UTILITY_H
