#ifndef INC_8008_ASSEMBLER_LEGACY_EVALUATE_H
#define INC_8008_ASSEMBLER_LEGACY_EVALUATE_H

#include <string_view>

class Context;

int legacy_evaluator(const Context& context, std::string_view arg);

#endif //INC_8008_ASSEMBLER_LEGACY_EVALUATE_H
