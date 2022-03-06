#ifndef INC_8008_ASSEMBLER_SECOND_PASS_H
#define INC_8008_ASSEMBLER_SECOND_PASS_H

#include <vector>

class Context;
class Files;
class Options;
class ParsedLine;
class SymbolTable;

void second_pass(const Context& context, Files& files, const SymbolTable& symbol_table,
                 const std::vector<ParsedLine>& parsed_lines);

#endif //INC_8008_ASSEMBLER_SECOND_PASS_H
