#ifndef INC_8008_ASSEMBLER_DATA_EXTRACTION_H
#define INC_8008_ASSEMBLER_DATA_EXTRACTION_H

#include "errors.h"

#include <string_view>

class Options;
class SymbolTable;

// Constants can be in different formats:
// Hex: 0xFF or 123h
// Oct: 123o or 123 with the -octal flag
// Bin: 11111111b
// Any string starting with an isalpha character denotes a symbol
//
// If the return value is negative, it's a reservation of uninitialized memory of the absolute value.
int decode_data(const Options& options, const SymbolTable& symbol_table,
                const std::string_view line, int* out_data);


class DataTooLong : public ExceptionWithReason
{
public:
    explicit DataTooLong();
};

class UnknownEscapeSequence : public ExceptionWithReason
{
public:
    explicit UnknownEscapeSequence(char escape);
};


#endif //INC_8008_ASSEMBLER_DATA_EXTRACTION_H
