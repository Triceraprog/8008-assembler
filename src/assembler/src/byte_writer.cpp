#include "byte_writer.h"
#include "evaluator.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <regex>

namespace
{
    const int MAX_BYTE_ON_LINE = 16;
    const int highest_address = 1024 * 16;
}

ByteWriter::ByteWriter(std::ostream& output, ByteWriter::WriteMode mode)
    : output(output), mode(mode)
{
    if (mode == BINARY)
    {
        program_memory.resize(highest_address);
    }
    current_line_content.reserve(MAX_BYTE_ON_LINE);
}

void ByteWriter::write_byte(int data, int address)
{
    if (address >= highest_address)
    {
        std::cerr << "address of data larger than " << highest_address - 1;
        exit(-1);
    }

    if (mode == BINARY)
    {
        program_memory[address] = (unsigned char) (data & 0xFF);
        return;
    }

    /* if jump in address, or line full */
    if ((address != (old_address + 1)) || (current_line_content.size() == MAX_BYTE_ON_LINE))
    {
        flush_hex_line();
        line_address = address;
    }
    current_line_content.push_back(data);
    old_address = address;
}

void ByteWriter::flush_hex_line()
{
    if (current_line_content.empty())
    {
        return;
    }

    output << std::hex << std::uppercase << std::setfill('0');
    output << ":";
    output << std::setw(2) << current_line_content.size();
    output << std::setw(4) << line_address;
    output << std::setw(2) << 0L;

    auto size_as_char = static_cast<unsigned char>(current_line_content.size());
    int checksum = size_as_char + (line_address & 0xFF) + ((line_address >> 8) & 0xFF);

    for (auto data_on_line : current_line_content)
    {
        checksum += data_on_line;
        output << std::setw(2) << static_cast<uint32_t>(data_on_line);
    }
    checksum = 0x100 - (checksum & 0xFF);

    output << std::setw(2) << checksum;
    output << '\n';

    current_line_content.clear();
}

void ByteWriter::write_end()
{
    if (mode == BINARY)
    {
        std::ranges::copy(program_memory, std::ostream_iterator<char>(output));
    }
    else
    {
        flush_hex_line();
        output << ":00000001FF\n";
    }
}
