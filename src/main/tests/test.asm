OK: EQU 1
NO: EQU 0


INC_HL: .macro
        .syntax     old
        INL
        JFZ skip
        INH
skip:
        .endmacro

        ; LD immediate specifying two registers
LD_IMM: .macro  r1,r2,imm
        .syntax new

        MOV r1,\HB\imm
        MOV r2,\LB\imm

        .endmacro

        ; MOV for a double register
MOVI:   .macro  reg,imm
        .syntax new

        MOV high(reg),\HB\imm
        MOV low(reg),\LB\imm

        .endmacro

INCR:   .macro registrer,value
        .syntax new

        INC low(register)
        JFZ skip
        INC high(register)
skip:
        .endmacro

        .INC_HL
        .LD_IMM H,L,$1234

    .syntax old
    LLH
    .context push
    .syntax new
    MOV L,H
    .context pop
    LLH

    .if NO
    This will no be parsed
    .endif

    .if OK
    LLH
    .else
    This will no be parsed
    .endif

    .inc_hl
    .inc_hl


    END



