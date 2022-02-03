#include "errors.h"

ParsingException::ParsingException(const std::exception& ex, int line_number, std::string& line) {
    reason = std::string{ex.what()} + " in line " + std::to_string(line_number) + ": " + line;
}
const char* ParsingException::what() const noexcept { return reason.c_str(); }
