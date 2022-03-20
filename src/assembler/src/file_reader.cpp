#include "file_reader.h"

#include <cassert>
#include <istream>

void FileReader::append(std::unique_ptr<std::istream> stream)
{
    input_streams.emplace_back(std::move(stream));

    auto line_iterator = std::istream_iterator<line>(*input_streams.back());
    line_iterators.push_back(line_iterator);

    if (line_count == -1)
    {
        line_count = 0;
        drop_front_empty_providers();
        extract_line_or_stop();
    }
}

FileReader::line_type FileReader::current_line_count() const { return line_count; }

void FileReader::advance()
{
    if (line_count == -1)
    {
        // The reader had previously consumed everything.
        return;
    }

    auto& current_line_it = line_iterators.front();
    assert(current_line_it != std::istream_iterator<line>());
    ++current_line_it;
    drop_front_empty_providers();
    extract_line_or_stop();
}

FileReader::Iterator FileReader::begin() { return Iterator{this}; }
FileReader::Iterator FileReader::end() { return Iterator{this, true}; }

FileReader::Iterator::Iterator(FileReader* file_reader, bool end)
    : file_reader{file_reader}, marker{end}
{
    if (!marker)
    {
        marker = this->file_reader->current_line_count();
    }
}

FileReader::Iterator::value_type FileReader::Iterator::operator*() const
{
    return file_reader->current_line();
}

FileReader::Iterator& FileReader::Iterator::operator++()
{
    file_reader->advance();
    marker = file_reader->current_line_count();
    return *this;
}

FileReader::Iterator FileReader::Iterator::operator++(int)
{
    Iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool operator==(const FileReader::Iterator& a, const FileReader::Iterator& b)
{
    return (a.file_reader == b.file_reader) && (a.marker == b.marker);
}

bool operator!=(const FileReader::Iterator& a, const FileReader::Iterator& b)
{
    return (a.file_reader != b.file_reader) || (a.marker != b.marker);
}

std::istream& operator>>(std::istream& stream, line& line)
{
    std::getline(stream, line);
    return stream;
}

void FileReader::drop_front_empty_providers()
{
    while (!line_iterators.empty() && (line_iterators.front() == std::istream_iterator<line>()))
    {
        line_iterators.pop_front();
        input_streams.pop_front();
    }
}

void FileReader::extract_line_or_stop()
{
    auto& current_line_it = line_iterators.front();
    if (!line_iterators.empty() && (current_line_it != std::istream_iterator<line>()))
    {
        latest_read_line = *current_line_it;
    }
    else
    {
        line_count = -1;
    }
}
