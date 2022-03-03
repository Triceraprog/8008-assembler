#ifndef INC_8008_ASSEMBLER_OPCODE_ACTION_H
#define INC_8008_ASSEMBLER_OPCODE_ACTION_H

#include "opcodes.h"
#include "options.h"
#include "symbol_table.h"

#include <memory>
#include <string>
#include <vector>

class ByteWriter;
class Listing;

class OpcodeAction
{
public:
    virtual void emit_byte_stream(ByteWriter& byte_writer) const = 0;
    virtual void emit_listing(Listing& listing, int line_number, std::string_view input_line) const = 0;

    virtual ~OpcodeAction() = default;
};

std::unique_ptr<OpcodeAction> create_opcode_action(const Options& options,
                                                   const SymbolTable& symbol_table, Opcode opcode,
                                                   int address,
                                                   const std::vector<std::string>& arguments);

class OpcodeActionNoArg : public OpcodeAction
{
public:
    explicit OpcodeActionNoArg(Opcode::OpcodeByteType opcode_byte, int address);

    void emit_byte_stream(ByteWriter& byte_writer) const override;
    void emit_listing(Listing& listing, int line_number,
                      std::string_view input_line) const override;

private:
    Opcode::OpcodeByteType opcode;
    int address;
};

class OpcodeActionOneByteArg : public OpcodeAction
{
public:
    OpcodeActionOneByteArg(const Options& options, const SymbolTable& symbol_table,
                           Opcode::OpcodeByteType opcode_byte, int address,
                           const std::vector<std::string>& arguments);

    void emit_byte_stream(ByteWriter& byte_writer) const override;
    void emit_listing(Listing& listing, int line_number,
                      std::string_view input_line) const override;

private:
    Opcode::OpcodeByteType opcode;
    int evaluated_argument;
    int address;
};

class OpcodeActionTwoByteArg : public OpcodeAction
{
public:
    OpcodeActionTwoByteArg(const Options& options, const SymbolTable& symbol_table,
                           Opcode::OpcodeByteType opcode_byte, int address,
                           const std::vector<std::string>& arguments);

    void emit_byte_stream(ByteWriter& byte_writer) const override;
    void emit_listing(Listing& listing, int line_number,
                      std::string_view input_line) const override;

private:
    Opcode::OpcodeByteType opcode;
    int evaluated_argument;
    int address;
};

class OpcodeActionInpOut : public OpcodeAction
{
public:
    OpcodeActionInpOut(const Options& options, const SymbolTable& symbol_table,
                       Opcode::OpcodeByteType opcode_byte, int address,
                       const std::vector<std::string>& arguments, std::string_view mnemonic);

    void emit_byte_stream(ByteWriter& byte_writer) const override;
    void emit_listing(Listing& listing, int line_number,
                      std::string_view input_line) const override;

private:
    Opcode::OpcodeByteType opcode;
    int address;
};

class OpcodeActionRst : public OpcodeAction
{
public:
    OpcodeActionRst(const Options& options, const SymbolTable& symbol_table,
                    Opcode::OpcodeByteType opcode_byte, int address,
                    const std::vector<std::string>& arguments);

    void emit_byte_stream(ByteWriter& byte_writer) const override;
    void emit_listing(Listing& listing, int line_number,
                      std::string_view input_line) const override;

private:
    Opcode::OpcodeByteType opcode;
    int address;
};

class ExpectedArgumentWithinLimits : public ExceptionWithReason
{
public:
    ExpectedArgumentWithinLimits(int limit, const std::string& content, int evaluated);
};

#endif //INC_8008_ASSEMBLER_OPCODE_ACTION_H
