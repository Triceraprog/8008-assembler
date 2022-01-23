#ifndef INC_8008_ASSEMBLER_FIRST_PASS_H
#define INC_8008_ASSEMBLER_FIRST_PASS_H

class SymbolTable;
class Files;
class Options;

void first_pass(const Options& options, SymbolTable& symbol_table, Files& files);

#endif //INC_8008_ASSEMBLER_FIRST_PASS_H
