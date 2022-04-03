#ifndef INC_8008_ASSEMBLER_LISTING_PASS_H
#define INC_8008_ASSEMBLER_LISTING_PASS_H

class Context;
class Listing;
class ParsedLineStorage;

void listing_pass(const Context& context, ParsedLineStorage& parsed_line_storage, Listing& listing);

#endif //INC_8008_ASSEMBLER_LISTING_PASS_H
