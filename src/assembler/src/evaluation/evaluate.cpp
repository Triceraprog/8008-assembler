#include "evaluate.h"

#include "legacy_evaluate.h"
#include "new_evaluate.h"

int evaluate(const Context& context, std::string_view arg)
{
    return legacy_evaluator(context, arg);
}

CannotFindSymbol::CannotFindSymbol(const std::string& symbol)
{
    reason = "cannot find symbol " + symbol;
}

UnknownOperation::UnknownOperation(const char operation)
{
    reason = "unknown operation ";
    reason.append(1, operation);
}

IllFormedExpression::IllFormedExpression() { reason = "the expression is ill-formed"; }

InvalidNumber::InvalidNumber(const std::string_view to_parse, std::string_view type_name,
                             std::string_view fail_reason)
{
    reason = "Tried to read '" + std::string{to_parse} + "' as " + std::string{type_name} +
             ". Argument is " + std::string{fail_reason};
}

ExpectedValue::ExpectedValue(std::string_view to_parse)
{
    reason = "Expected value, found '" + std::string(to_parse) + "'";
}
