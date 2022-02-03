#ifndef INC_8008_ASSEMBLER_EVALUATOR_H
#define INC_8008_ASSEMBLER_EVALUATOR_H

#include "errors.h"

#include <exception>
#include <string>
#include <string_view>

class Options;
class SymbolTable;

int evaluate_argument(const Options& options, const SymbolTable& symbol_table,
                      std::string_view arg);

class CannotFindSymbol : public ExceptionWithReason
{
public:
    explicit CannotFindSymbol(const std::string& symbol);
};

class UnknownOperation : public ExceptionWithReason
{
public:
    explicit UnknownOperation(char operation);
};

class IllFormedExpression : public ExceptionWithReason
{
public:
    explicit IllFormedExpression();
};

class InvalidNumber : public ExceptionWithReason
{
public:
    explicit InvalidNumber(std::string_view to_parse, std::string_view type_name,
                           std::string_view fail_reason);
};

class ExpectedValue : public ExceptionWithReason
{
public:
    explicit ExpectedValue(std::string_view to_parse);
};

#endif //INC_8008_ASSEMBLER_EVALUATOR_H
