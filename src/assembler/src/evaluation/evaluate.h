#ifndef INC_8008_ASSEMBLER_EVALUATE_H
#define INC_8008_ASSEMBLER_EVALUATE_H

#include "errors.h"

class Context;

int evaluate(const Context& context, std::string_view arg);

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

#endif //INC_8008_ASSEMBLER_EVALUATE_H
