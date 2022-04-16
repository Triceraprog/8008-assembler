#ifndef INC_8008_ASSEMBLER_CONTEXT_STACK_H
#define INC_8008_ASSEMBLER_CONTEXT_STACK_H

#include <memory>
#include <stack>

class Context;
class Options;

class ContextStack
{
public:
    explicit ContextStack(const Options& options);

    void push();
    void pop();

    std::shared_ptr<Context> get_current_context();
    std::shared_ptr<Context> get_current_context() const;

private:
    std::stack<std::shared_ptr<Context>> stack;
};

#endif //INC_8008_ASSEMBLER_CONTEXT_STACK_H
