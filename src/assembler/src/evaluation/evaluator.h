#ifndef INC_8008_ASSEMBLER_EVALUATOR_H
#define INC_8008_ASSEMBLER_EVALUATOR_H

#include "context.h"
#include "errors.h"

#include <exception>
#include <string>
#include <string_view>

class Options;
class SymbolTable;

int evaluate_argument(const Context& context, std::string_view arg);

#endif //INC_8008_ASSEMBLER_EVALUATOR_H
