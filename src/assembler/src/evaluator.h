#ifndef INC_8008_ASSEMBLER_EVALUATOR_H
#define INC_8008_ASSEMBLER_EVALUATOR_H

#include <string_view>

class Options;
class SymbolTable;

int evaluate_argument(Options& options, const SymbolTable& symbol_table, int current_line_count,
                      std::string_view arg);

#endif //INC_8008_ASSEMBLER_EVALUATOR_H
