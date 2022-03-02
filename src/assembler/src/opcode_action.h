#ifndef INC_8008_ASSEMBLER_OPCODE_ACTION_H
#define INC_8008_ASSEMBLER_OPCODE_ACTION_H

#include "opcodes.h"
#include "options.h"
#include "symbol_table.h"

#include <memory>
#include <string>
#include <vector>

class ByteWriter;

class OpcodeAction
{
public:
    virtual ~OpcodeAction() = default;
};

std::unique_ptr<OpcodeAction> create_opcode_action(const Options& options,
                                                   const SymbolTable& symbol_table,
                                                   unsigned char opcode, int address,
                                                   const std::vector<std::string>& arguments,
                                                   std::string_view mnemonic);

class OpcodeActionNoArg : public OpcodeAction
{
public:
    explicit OpcodeActionNoArg(unsigned char opcode, int address);

    void emit_byte_stream(ByteWriter& byte_writer) const;

private:
    Opcode::OpcodeByteType opcode;
    int address;
};

class OpcodeActionOneByteArg : public OpcodeAction
{
public:
    OpcodeActionOneByteArg(const Options& options, const SymbolTable& symbol_table,
                           unsigned char opcode, int address,
                           const std::vector<std::string>& arguments);

    void emit_byte_stream(ByteWriter& byte_writer) const;

private:
    Opcode::OpcodeByteType opcode;
    int evaluated_argument;
    int address;
};

class OpcodeActionTwoByteArg : public OpcodeAction
{
public:
    OpcodeActionTwoByteArg(const Options& options, const SymbolTable& symbol_table,
                           unsigned char opcode, int address,
                           const std::vector<std::string>& arguments);

    void emit_byte_stream(ByteWriter& byte_writer) const;

private:
    Opcode::OpcodeByteType opcode;
    int evaluated_argument;
    int address;
};

class OpcodeActionInpOut : public OpcodeAction
{
public:
    OpcodeActionInpOut(const Options& options, const SymbolTable& symbol_table,
                       unsigned char opcode, int address, const std::vector<std::string>& arguments,
                       std::string_view mnemonic);

    void emit_byte_stream(ByteWriter& byte_writer) const;

private:
    Opcode::OpcodeByteType opcode;
    int address;
};

class OpcodeActionRst : public OpcodeAction
{
public:
    OpcodeActionRst(const Options& options, const SymbolTable& symbol_table, unsigned char opcode,
                    int address, const std::vector<std::string>& arguments);

    void emit_byte_stream(ByteWriter& byte_writer) const;

private:
    Opcode::OpcodeByteType opcode;
    int address;
};

#endif //INC_8008_ASSEMBLER_OPCODE_ACTION_H
