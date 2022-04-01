#include "file_utility.h"

#include "files.h"

#include <memory>

void Utility::append_file_by_name(FileReader& file_reader, const std::string& filename)
{
    auto stream = std::make_unique<std::ifstream>(filename.c_str());

    if (stream->fail())
    {
        throw CannotOpenFile(filename, "input file");
    }

    file_reader.append(std::move(stream), std::string_view());
}

void Utility::insert_file_by_name(FileReader& file_reader, const std::string& filename)
{
    auto stream = std::make_unique<std::ifstream>(filename.c_str());

    if (stream->fail())
    {
        throw CannotOpenFile(filename, "include file");
    }

    file_reader.insert_now(std::move(stream), std::string_view());
}
