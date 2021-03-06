#ifndef INC_8008_ASSEMBLER_UTILS_H
#define INC_8008_ASSEMBLER_UTILS_H

#include <string_view>

bool ci_equals(const std::string_view& lhs, const std::string_view& rhs);
std::string_view trim_string(std::string_view str);

#endif //INC_8008_ASSEMBLER_UTILS_H
