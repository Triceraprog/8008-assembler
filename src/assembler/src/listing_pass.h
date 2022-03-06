#ifndef INC_8008_ASSEMBLER_LISTING_PASS_H
#define INC_8008_ASSEMBLER_LISTING_PASS_H

#include <vector>

class Context;
class Files;
class Listing;
class Options;
class ParsedLine;
class SymbolTable;

void listing_pass(const Context& context, const Options& options, const SymbolTable& symbol_table,
                  Files& files, const std::vector<ParsedLine>& parsed_lines, Listing& listing);

#endif //INC_8008_ASSEMBLER_LISTING_PASS_H
