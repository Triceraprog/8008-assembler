#include "errors.h"
#include "options.h"
#include "utils.h"

const char* ExceptionWithReason::what() const noexcept { return reason.c_str(); }

ParsingException::ParsingException(const std::exception& ex, int line_number, std::string& line)
{
    reason = std::string{ex.what()} + " in line " + std::to_string(line_number) + ": " + line;
}
