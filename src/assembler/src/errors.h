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
    explicit ParsingException(const std::exception& ex, std::size_t line_number,
                              const std::string& line);
    explicit ParsingException(const std::exception& ex, std::size_t line_number,
                              const std::string_view context_name, const std::string& line);
};

class InternalError : public ExceptionWithReason
{
public:
    explicit InternalError(const std::string& line);
};

#endif //INC_8008_ASSEMBLER_ERRORS_H
