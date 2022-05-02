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
value:  EQU imm
        MOV r1,\HB\value
        MOV r2,\LB\value
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
        .incr hl

        .context push
TEST:
        JMP TEST
        .context pop
TEST:
        JMP TEST
        END
