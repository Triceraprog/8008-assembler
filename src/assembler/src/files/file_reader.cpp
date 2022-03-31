#include "file_reader.h"

#include <cassert>
#include <istream>

void FileReader::append(std::unique_ptr<std::istream> stream)
{
    input_streams.emplace_back(std::move(stream));

    auto line_iterator = std::istream_iterator<line>(*input_streams.back());
    line_iterators.push_back(line_iterator);
    current_line_counts.push_back(1);

    if (exhausted)
    {
        exhausted = false;
        drop_front_empty_providers();
        extract_line_or_stop();
    }
}

void FileReader::insert_now(std::unique_ptr<std::istream> stream)
{
    // As insert interrupts the current stream, the new stream is placed
    // in front of the streams. After it is consumed, it will naturally
    // go back to the previous streams, like in a stack.
    input_streams.emplace_front(std::move(stream));

    auto line_iterator = std::istream_iterator<line>(*input_streams.front());
    line_iterators.push_front(line_iterator);
    current_line_counts.push_front(1);

    interrupted = true; // Will not work if interrupted by an empty stream
    exhausted = false;

    drop_front_empty_providers();
}

bool FileReader::content_exhausted() const { return exhausted; }

void FileReader::advance()
{
    if (exhausted)
    {
        // The reader had previously consumed everything.
        return;
    }

    auto& current_line_it = line_iterators.front();
    assert(current_line_it != std::istream_iterator<line>());
    if (interrupted)
    {
        // A new stream was added in front.
        // In that case, advancing means advancing the interrupted stream and reading
        // the first line of the new stream.
        assert(line_iterators.size() > 1);
        ++line_iterators[1];
        ++current_line_counts[1];
        interrupted = false;
    }
    else
    {
        ++current_line_it;
        current_line_counts.front() += 1;
    }
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
        marker = this->file_reader->content_exhausted();
    }
}

FileReader::Iterator::value_type FileReader::Iterator::operator*() const
{
    return file_reader->current_line();
}

FileReader::Iterator& FileReader::Iterator::operator++()
{
    file_reader->advance();
    marker = file_reader->content_exhausted();
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
        current_line_counts.pop_front();
    }
}

void FileReader::extract_line_or_stop()
{
    auto& current_line_it = line_iterators.front();
    if (!line_iterators.empty() && (current_line_it != std::istream_iterator<line>()))
    {
        latest_read_line = *current_line_it;
        current_line_count = current_line_counts.front();
    }
    else
    {
        exhausted = true;
    }
}

size_t FileReader::get_line_number() const
{
    return current_line_count;
}
