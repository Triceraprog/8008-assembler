#ifndef INC_8008_ASSEMBLER_BYTE_WRITER_H
#define INC_8008_ASSEMBLER_BYTE_WRITER_H

#include "errors.h"

#include <cstdio>
#include <ostream>
#include <vector>

class ByteWriter
{
public:
    enum WriteMode
    {
        BINARY,
        HEX,
    };

    ByteWriter(std::ostream& output, WriteMode mode);

    void write_byte(int data, int address);
    void write_end();

private:
    void flush_hex_line();

    std::ostream& output;
    WriteMode mode;
    std::vector<unsigned char> program_memory;
    std::vector<int> current_line_content;
    int old_address = -1000;
    int line_address{};
};

class AddressTooHigh : public ExceptionWithReason
{
public:
    explicit AddressTooHigh(int faulty_address);
};

#endif //INC_8008_ASSEMBLER_BYTE_WRITER_H
