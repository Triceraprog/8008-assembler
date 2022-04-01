#include "errors.h"
#include "utils.h"

const char* ExceptionWithReason::what() const noexcept { return reason.c_str(); }

ParsingException::ParsingException(const std::exception& ex, std::size_t line_number,
                                   const std::string& line)
{
    reason = std::string{ex.what()} + " in line " + std::to_string(line_number) + ": " + line;
}

ParsingException::ParsingException(const std::exception& ex, std::size_t line_number,
                                   const std::string_view context_name, const std::string& line)
{
    reason = std::string{ex.what()} + " in line " + std::string{context_name} +
             "::" + std::to_string(line_number) + ": " + line;
}

InternalError::InternalError(const std::string& line) { reason = "internal error: " + line; }
