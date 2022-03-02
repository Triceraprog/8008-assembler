#ifndef INC_8008_ASSEMBLER_SECOND_PASS_H
#define INC_8008_ASSEMBLER_SECOND_PASS_H

#include "errors.h"

#include <vector>

class Options;
class SymbolTable;
class Files;
class Listing;
class ParsedLine;

void second_pass(const Options& options, const SymbolTable& symbol_table, Files& files,
                 std::vector<ParsedLine>& parsed_lines, Listing& listing);

#endif //INC_8008_ASSEMBLER_SECOND_PASS_H
