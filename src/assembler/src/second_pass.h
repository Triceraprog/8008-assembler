#ifndef INC_8008_ASSEMBLER_SECOND_PASS_H
#define INC_8008_ASSEMBLER_SECOND_PASS_H

#include <ostream>

class Options;
class ParsedLine;
class SymbolTable;
class ParsedLineStorage;

void second_pass(const Options& global_options, std::ostream& output_stream,
                 ParsedLineStorage& parsed_line_storage);

#endif //INC_8008_ASSEMBLER_SECOND_PASS_H
