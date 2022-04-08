#ifndef INC_8008_ASSEMBLER_LISTING_PASS_H
#define INC_8008_ASSEMBLER_LISTING_PASS_H

class Listing;
class Options;
class ParsedLineStorage;

void listing_pass(const Options& global_options, ParsedLineStorage& parsed_line_storage, Listing& listing);

#endif //INC_8008_ASSEMBLER_LISTING_PASS_H
