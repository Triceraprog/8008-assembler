#ifndef INC_8008_ASSEMBLER_FILE_READER_H
#define INC_8008_ASSEMBLER_FILE_READER_H

#include <cstdint>
#include <deque>
#include <memory>
#include <string>

struct line : public std::string
{
    using std::string::string;
};

std::istream& operator>>(std::istream& stream, line& line);

class FileReader
{
public:
    using line_type = std::int_least32_t;

    class Iterator
    {
    public:
        using iterator_category [[maybe_unused]] = std::input_iterator_tag;
        using difference_type [[maybe_unused]] = int;
        using value_type = std::string;
        using pointer = value_type*;
        using reference = value_type&;

        value_type operator*() const;
        Iterator& operator++();
        Iterator operator++(int);

        friend bool operator==(const Iterator& a, const Iterator& b);
        friend bool operator!=(const Iterator& a, const Iterator& b);

    private:
        explicit Iterator(FileReader* file_reader, bool end = false);

        bool marker;
        FileReader* file_reader;

        friend FileReader;
    };

    Iterator begin();;
    Iterator end();;

    void append(std::unique_ptr<std::istream> stream);

private:
    std::deque<std::unique_ptr<std::istream>> input_streams;
    std::deque<std::istream_iterator<line>> line_iterators;
    bool exhausted{true};
    line latest_read_line;

    [[nodiscard]] bool content_exhausted() const;

    void advance();
    void drop_front_empty_providers();
    void extract_line_or_stop();

    [[nodiscard]] const std::string& current_line() const { return latest_read_line; }
};

#endif //INC_8008_ASSEMBLER_FILE_READER_H
