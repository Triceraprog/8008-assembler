#ifndef INC_8008_ASSEMBLER_EVALUATOR_H
#define INC_8008_ASSEMBLER_EVALUATOR_H

#include <exception>
#include <string>
#include <string_view>

class Options;
class SymbolTable;

int evaluate_argument(const Options& options, const SymbolTable& symbol_table,
                      int current_line_count, std::string_view arg);

class CannotFindSymbol : public std::exception
{
public:
    explicit CannotFindSymbol(const std::string& symbol);
    [[nodiscard]] const char* what() const noexcept override;

private:
    std::string reason;
};

#endif //INC_8008_ASSEMBLER_EVALUATOR_H
