#include "byte_writer.h"
#include "evaluator.h"
#include "files.h"
#include "line_tokenizer.h"
#include "opcodes.h"
#include "options.h"
#include "symbol_table.h"
#include "utils.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <regex>

#define MAX_BYTE_ON_LINE 16

const int highest_address = 1024 * 16;

ByteWriter::ByteWriter(FILE* ofp, ByteWriter::WriteMode mode) : ofp(ofp), mode(mode)
{
    if (mode == BINARY)
    {
        program_memory.resize(highest_address);
    }
    current_line_content.reserve(MAX_BYTE_ON_LINE * 2);
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

    /* Intel HEX format */
    /* if jump in address, or line full, or end of data, write line */
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

    auto size_as_char = static_cast<unsigned char>(current_line_content.size());
    fprintf(ofp, ":%02X%04X%02X", size_as_char, line_address, 0);
    int checksum = size_as_char + (line_address & 0xFF) + ((line_address >> 8) & 0xFF);
    for (auto data_on_line : current_line_content)
    {
        checksum += data_on_line;
        fprintf(ofp, "%02X", data_on_line);
    }
    checksum = 0x100 - (checksum & 0xFF);
    fprintf(ofp, "%02X\n", checksum);
    current_line_content.clear();
}

void ByteWriter::write_end()
{
    if (mode == BINARY)
    {
        fwrite(program_memory.data(), 16384, 1, ofp);
    }
    else
    {
        flush_hex_line();
        fprintf(ofp, ":00000001FF\n");
    }
}
