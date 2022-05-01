#ifndef INC_8008_ASSEMBLER_INSTRUCTION_H
#define INC_8008_ASSEMBLER_INSTRUCTION_H

#include "errors.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class ByteWriter;
class Context;
class ContextStack;
class FileReader;
class Listing;
class Options;
class SymbolTable;

enum class InstructionEnum
{
    EMPTY,
    EQU,
    END,
    CPU,
    ORG,
    DATA,
    INCLUDE,
    SYNTAX,
    CONTEXT,
    IF,
    ELSE,
    ENDIF,
    MACRO,
    ENDMACRO,
    MACRO_CALL,
    OTHER,
};

class Instruction
{
public:
    Instruction(const Context& context, const std::string& label, const std::string& opcode,
                const std::vector<std::string>& arguments, FileReader& file_reader);

    [[nodiscard]] std::optional<int> get_value_for_label(const Context& context, int address) const;

    [[nodiscard]] int first_pass(ContextStack& context_stack, int address) const;

    void second_pass(const Context& context, ByteWriter& writer, int address) const;

    void listing_pass(Listing& listing, const std::string& input_line, uint32_t line_number,
                      int address) const;

    class InstructionAction
    {
    public:
        virtual ~InstructionAction() = default;

        // This is used when there's a need of evaluation of an expression instruction like
        // ORG or EQU, before the first pass.
        // The default is to return the given address.
        [[nodiscard]] virtual std::optional<int> evaluate_fixed_address(const Context& context,
                                                                        int address) const;

        // This is used by the first pass to know how many bytes are needed for this instruction.
        // The default is a need of 0, and so returns the given address.
        [[nodiscard]] virtual int advance_address(const Context& context, int address) const;

        // Some instruction can modify the context_stack
        virtual void update_context_stack(ContextStack& context_stack) const;

        // If the construction needs information from the first pass, then it is constructed
        // at build time.
        virtual void build(const Context& context, int address);

        // Write the bytes for the instruction to the ByteWriter.
        // Currently, also emits the listing, it will have to go
        virtual void write_bytes(const Context& context, ByteWriter& writer, int address) const;

        virtual void write_listing(Listing& listing, const std::string& input_line,
                                   uint32_t line_number, int address) const;
    };

private:
    std::unique_ptr<InstructionAction> action;
};

InstructionEnum instruction_to_enum(std::string_view opcode);

class InvalidCPU : public ExceptionWithReason
{
public:
    InvalidCPU();
};

class InvalidSyntax : public ExceptionWithReason
{
public:
    InvalidSyntax();
};

class InvalidContextAction : public ExceptionWithReason
{
public:
    InvalidContextAction();
};

class InvalidConditional : public ExceptionWithReason
{
public:
    explicit InvalidConditional(std::string_view conditional_name);
};

class MissingArgument : public ExceptionWithReason
{
public:
    explicit MissingArgument(std::string_view instruction);
};

class UndefinedMacro : public ExceptionWithReason
{
public:
    explicit UndefinedMacro(std::string_view macro_name);
};

class InvalidEndmacro : public ExceptionWithReason
{
public:
    InvalidEndmacro();
};

class WrongNumberOfParameters : public ExceptionWithReason
{
public:
    WrongNumberOfParameters(std::string_view macro_name, size_t expected, size_t got);
};

#endif //INC_8008_ASSEMBLER_INSTRUCTION_H
