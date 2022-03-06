#ifndef INC_8008_ASSEMBLER_INSTRUCTION_H
#define INC_8008_ASSEMBLER_INSTRUCTION_H

#include "errors.h"
#include "opcodes/opcodes.h"

#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

class Context;
class Options;
class SymbolTable;
class ByteWriter;
class Listing;
class LineTokenizer;

enum class InstructionEnum
{
    EMPTY,
    EQU,
    END,
    CPU,
    ORG,
    DATA,
    OTHER,
};

class Instruction
{
public:
    explicit Instruction(const Context& context, const std::string& opcode,
                         std::vector<std::string> arguments);

    [[nodiscard]] int get_evaluation(const Context& context, const Options& options,
                                     const SymbolTable& symbol_table, int current_address) const;

    [[nodiscard]] int first_pass(const Context& context, int current_address) const;

    void second_pass(const Context& context, ByteWriter& writer, const int address) const;

    void listing_pass(Listing& listing, const std::string& input_line, int line_number,
                      int address) const;

    class InstructionAction
    {
    public:
        virtual ~InstructionAction() = default;

        // This is used when there's a need of evaluation of an expression instruction like
        // ORG or EQU, before the first pass.
        // The default is to return the given address.
        [[nodiscard]] virtual int evaluate_fixed_address(const Context& context,
                                                         int current_address) const;

        // This is used by the first pass to know how many bytes are needed for this instruction.
        // The default is a need of 0, and so returns the given address.
        [[nodiscard]] virtual int advance_address(const Context& context,
                                                  int current_address) const;

        // If the construction needs information from the first pass, then it is constructed
        // at build time.
        virtual void build(const Context& context, int address);

        // Write the bytes for the instruction to the ByteWriter.
        // Currently, also emits the listing, it will have to go
        virtual void write_bytes(const Context& context, ByteWriter& writer, int address) const;

        virtual void write_listing(Listing& listing, const std::string& input_line, int line_number,
                                   int address) const;
    };

private:
    const std::vector<std::string> arguments;
    std::unique_ptr<InstructionAction> action;
};

InstructionEnum instruction_to_enum(std::string_view opcode);

class InvalidCPU : public ExceptionWithReason
{
public:
    InvalidCPU();
};

class MissingArgument : public ExceptionWithReason
{
public:
    MissingArgument(std::string_view instruction);
};


#endif //INC_8008_ASSEMBLER_INSTRUCTION_H
