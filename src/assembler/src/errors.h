#ifndef INC_8008_ASSEMBLER_ERRORS_H
#define INC_8008_ASSEMBLER_ERRORS_H

#include <exception>
#include <string>

class ParsingException : std::exception
{
public:
    explicit ParsingException(const std::exception& ex, int line_number, std::string & line);
    [[nodiscard]] const char* what() const noexcept override;

private:
    std::string reason;
};

#endif //INC_8008_ASSEMBLER_ERRORS_H
