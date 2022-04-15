OK: EQU 1
NO: EQU 0

    .macro INC_HL
    INL
    JFZ skip
    INH
skip:
    .endmacro

;     .macro INC,register
;     INC low(register)
;     JFZ skip
;     INC high(register)
; skip:
;     .endmacro

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

