#ifndef INC_8008_ASSEMBLER_OPCODE_ACTION_INPOUT_H
#define INC_8008_ASSEMBLER_OPCODE_ACTION_INPOUT_H

#include "opcode_action.h"

class OpcodeActionInpOut : public OpcodeAction
{
public:
    OpcodeActionInpOut(const Options& options, const SymbolTable& symbol_table,
                       Opcode::OpcodeByteType opcode_byte, int address,
                       const std::vector<std::string>& arguments, std::string_view mnemonic);

    void emit_byte_stream(ByteWriter& byte_writer) const override;
    void emit_listing(Listing& listing, int line_number, std::string_view input_line,
                      bool single_byte) const override;

private:
    Opcode::OpcodeByteType opcode;
    int address;
};

#endif //INC_8008_ASSEMBLER_OPCODE_ACTION_INPOUT_H
