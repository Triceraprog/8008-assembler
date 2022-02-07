#ifndef INC_8008_ASSEMBLER_SECOND_PASS_H
#define INC_8008_ASSEMBLER_SECOND_PASS_H

class Options;
class SymbolTable;
class Files;
class Listing;

void second_pass(const Options& options, const SymbolTable& symbol_table, Files& files,
                 Listing& listing);

#endif //INC_8008_ASSEMBLER_SECOND_PASS_H
