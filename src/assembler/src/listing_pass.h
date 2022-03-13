#ifndef INC_8008_ASSEMBLER_LISTING_PASS_H
#define INC_8008_ASSEMBLER_LISTING_PASS_H

#include <vector>

class Context;
class Listing;
class ParsedLine;

void listing_pass(const Context& context, const std::vector<ParsedLine>& parsed_lines,
                  Listing& listing);

#endif //INC_8008_ASSEMBLER_LISTING_PASS_H
