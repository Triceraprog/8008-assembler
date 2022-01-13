VALUE:  EQU 012
LONG:   EQU 050#050

    ; Comment
    ORG 000#000

    ; Index Register Instructions
    LAA
    LMB
    LCM
    LDI 030
    LEI 0xFF
    LHI \HB\SOMEDATA
    LLI \LB\SOMEDATA
    LMI 123
    IND
    DCE

    ; Accumulator group
    ADC
    ADM
    ADI 0x00
    ACD
    ACM
    ACI 022

    SUH
    SUM
    SUI 033
    SBB
    SBM
    SBI 0x11

    NDA
    NDM
    NDI 0x00
    XRB
    XRM
    XRI 10
    ORC
    ORM
    ORI 0x11
    CPD
    CPM
    CPI 111

    RLC
    RRC
    RAL
    RAR

    ; Program Counter and Stack Control Instructions
LOOP:
    JMP LOOP
    JFC LOOP
    JTC LOOP
    JFZ LOOP
    JTZ LOOP
    JFS LOOP
    JTS LOOP
    JFP LOOP
    JTP LOOP

    CAL ROUTINE
    CFC ROUTINE
    CTC ROUTINE
    CFZ ROUTINE
    CTZ ROUTINE
    CFS ROUTINE
    CTS ROUTINE
    CFP ROUTINE
    CTP ROUTINE

    ; RST 0 ; Not supported at the moment

ROUTINE:
    RET
    RFC
    RTC
    RFZ
    RTZ
    RFS
    RTS
    RFP
    RTP

    ; Input Ouput Instructions
    INP 2
    OUT 000

    ; Machine Instruction
    HLT

SOMEDATA:
    DATA 000,001,002,003,004    ; End of line comment
    DATA *2
