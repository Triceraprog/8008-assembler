#ifndef INC_8008_ASSEMBLER_BYTE_WRITER_H
#define INC_8008_ASSEMBLER_BYTE_WRITER_H

#include <cstdio>
#include <vector>

class ByteWriter
{
public:
    enum WriteMode
    {
        BINARY,
        HEX,
    };

    ByteWriter(FILE* ofp, WriteMode mode);

    void write_byte(int data, int address);
    void write_end();

private:
    void flush_hex_line();

    FILE* ofp;
    WriteMode mode;
    std::vector<unsigned char> program_memory;
    std::vector<int> current_line_content;
    int old_address = -1000;
    int line_address{};
};

#endif //INC_8008_ASSEMBLER_BYTE_WRITER_H
