#include "file_reader.h"

#include <cassert>
#include <istream>

FileReader::ReaderContext::ReaderContext(std::unique_ptr<std::istream>&& stream,
                                         std::string_view name_tag)
    : input_stream{std::move(stream)}, current_line_count{1}, name_tag{name_tag}
{
    line_iterator = std::istream_iterator<line>{*input_stream};
}

void FileReader::append(std::unique_ptr<std::istream> stream, std::string_view name_tag)
{
    contexts.emplace_back(std::move(stream), name_tag);

    if (exhausted)
    {
        exhausted = false;
        drop_front_empty_providers();
        extract_line_or_stop();
    }
}

void FileReader::insert_now(std::unique_ptr<std::istream> stream, std::string_view name_tag)
{
    if (contexts.empty())
    {
        return append(std::move(stream), name_tag);
    }

    // As insert interrupts the current stream, the new stream is placed
    // in front of the streams. After it is consumed, it will naturally
    // go back to the previous streams, like in a stack.
    auto stacked_name_tag = contexts.front().name_tag + "::" + std::string{name_tag};
    contexts.emplace_front(std::move(stream), stacked_name_tag);

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

    assert(contexts.front().line_iterator != std::istream_iterator<line>());
    if (interrupted)
    {
        // A new stream was added in front.
        // In that case, advancing means advancing the interrupted stream and reading
        // the first line of the new stream.
        assert(contexts.size() > 1);
        ++contexts[1].line_iterator;
        ++contexts[1].current_line_count;
        interrupted = false;
    }
    else
    {
        ++contexts[0].line_iterator;
        ++contexts[0].current_line_count;
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
    while (!contexts.empty() && (contexts.front().line_iterator == std::istream_iterator<line>()))
    {
        contexts.pop_front();
    }
}

void FileReader::extract_line_or_stop()
{
    if (!contexts.empty() && (contexts.front().line_iterator != std::istream_iterator<line>()))
    {
        latest_read_line = *contexts.front().line_iterator;
        current_line_count = contexts.front().current_line_count;
        current_name_tag = contexts.front().name_tag;
    }
    else
    {
        exhausted = true;
    }
}

std::size_t FileReader::get_line_number() const { return current_line_count; }

std::string FileReader::get_name_tag() const { return current_name_tag; }
