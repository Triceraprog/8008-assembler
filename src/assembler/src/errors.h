#ifndef INC_8008_ASSEMBLER_ERRORS_H
#define INC_8008_ASSEMBLER_ERRORS_H

#include <exception>
#include <string>
#include <string_view>

class ExceptionWithReason : public std::exception
{
public:
    [[nodiscard]] const char* what() const noexcept override;

protected:
    std::string reason;
};

class ParsingException : public ExceptionWithReason
{
public:
    explicit ParsingException(const std::exception& ex, int line_number, std::string& line);
};

#endif //INC_8008_ASSEMBLER_ERRORS_H
