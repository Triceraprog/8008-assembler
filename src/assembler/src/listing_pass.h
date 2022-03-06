#ifndef INC_8008_ASSEMBLER_LISTING_PASS_H
#define INC_8008_ASSEMBLER_LISTING_PASS_H

#include <vector>

class Context;
class Files;
class Listing;
class Options;
class ParsedLine;

void listing_pass(const Context& context, Files& files, const std::vector<ParsedLine>& parsed_lines,
                  Listing& listing);

#endif //INC_8008_ASSEMBLER_LISTING_PASS_H
