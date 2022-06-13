# 8008-Assembler

## Command line

    Usage: as-8008 [options] infile
        where <infile> is assembly code file, extension defaults to .asm
        and options include...
        -v          verbose output
        -nl         no list (default is to make .lst file.)
        -d          debug assembler (extra output)
        -bin        makes output binary ROM file, otherwise intel hex
        -octal      makes unidentified 3-digit numbers octal (default decimal)
        -single     makes .lst file single byte per line, otherwise 3/line.
        -as8        as8 compatibility mode.
        -markascii  makes highest bit in ascii bytes a one (mark).
        -syntax=new default parsing is with new syntax mnemonics.
        -o          the next argument is the output filename base.

The command line can take several options followed by one or several input files.
These input files must be 8008 assembly files with the syntax described later
in this document.

By default, the result is an assembled result in Intel Hex format, with the '.hex'
extension.

If no extension is given to a file, a '.asm' extension will be added.

### Options

#### -v: verbose

With this flag, more messages are sent to the standard output.

#### -nl: no list

The listing file is not emitted. If not set, a listing file with the '.lst' extension
is emitted, containing the result of the assembling, in textual form.

#### -d: debug output

With this flag, internal messages are sent to the standard output. Mainly useful
for debugging purposes.

#### -bin: output binary file

With this flag, the result of the assembling will be sent to a file in binary
form, with the '.bin' extension.

The file is 16384 byte long and represents the full content of the addressable
memory space for the 8008.

This flag causes the Intel Hex format file not to be written.

#### -octal: default to octal

3-digit numbers with no specifier are considered octal. 

#### -single: one single byte per line

Causes the listing file to be formatted to have only one result byte by
line. They are otherwise packed 3 by 3 when it makes sens.

#### -as8: as8 compatibility mode

Causes the assembler to:

* use the simple arithmetic solver, with no operator precedence.
* limit the 'DATA' pseudo-op to 12 bytes per input line.

#### -markascii: Mark-8 compatibility mode

All ASCII data high bits are set to 1, as needed by the Mark-8 computer.

#### -syntax=[new|old]: sets default parsing.

By default, the assembler uses the old Intel syntax (e.g.: understands `LAA`).
With `-syntax=new`, the default  syntax is turned to the new Intel
syntax (e.g.: understands `MOV A,A`).

Because some mnemonics are ambiguous, the assembler must be in one of the two
syntax at any moment. A command to switch from one to the other is available
(see further).

#### -o: specify output filename base.

Following the `-o` flag must be a name that will serve as the filename base for
produced files (listing and assembly output).

## Assembly Syntax

By default, the assembler understand the old Intel Syntax. The new syntax can
be specified on command line or with assembly commands (see further).

Opcode, pseudo-opcode and commands are case insensitive.

### Format

Labels start on the first column and are immediately followed by a column.

```asm
label:
```

Opcodes and command start anywhere after the first column and are separated 
from their arguments by at least one blank space.

```asm
    JMP 0x0100
```

Comments start by a semi-column and can be anywhere on the line. Everything after
is ignored.

```asm
; This is a comment
```

### RST

The `RST` opcode parsing understands both addressing forms. It accept both
a number from `0` to `7` or an actual restart address (`0x00`, `0x08`,... `0x38`).

```asm
    RST 1 ; gives the same code as
    RST 8
```

### MAS

Specific `Micral N` instruction for `Mask all interruptions`. It is coded as `LCC`.

### DMS

Specific `Micral N` instruction for `DeMask all interruptions`. It is coded as `LLL`.

### REI

Specific `Micral N` instruction for `Return from Interruption`. It is coded as a
specific variation of `RET` (octal 037). It's both a `RET` and a `DMS`.

## Assembly Commands

The assembler understands some specific commands, or pseudo opcodes.

### ORG

`ORG` specifies the starting address for the following commands.

The assembler doesn't verify any coherency, and you can have overlaps in produced
binary code if not careful.

```asm
    ORG 0x0100
```

### EQU

Provides equivalence between a label and the value in parameter of `EQU`.

`EQU` must be used in a line where a label was specified. The name of this
label then becomes a symbol for the rest of the assembly. It can be use in
any arithmetic expression.

```asm
count:  EQU 32  ; count is now a symbol with a decimal value of 32.
```


### END

`END` states the end of the input file. It is completely ignored.

```asm
    END     ; has absolutely no effect.
```

### CPU

`CPU` specified the kind of cpu. It must be either `8008` or `i8008`. It
is provided only for compatibility. The assembler doesn't know any other
CPU type.

```asm
    CPU 8008
```


### DATA/DB

`DATA` and `DB` are equivalent pseudo opcodes that must be followed by
a comma separated list of data that will be converted to successive bytes.

The data can any expression understood by the solver (see further), or
character strings enclosed by double quotes and containing ASCII characters.

```asm
    DATA 32,33,34,35
    DATA '\n'
    DB   "Hello!",0     ; DB is an alias for DATA
```


### .INCLUDE

Followed by a filename, the `.include` command will insert the content of the
specified file in place.

Included files can themselves include other files.

```asm
    .INCLUDE common.asm
```


### .SYNTAX

The command specifies the syntax used by the assembler starting the next line.
The two valid arguments are `NEW` or `OLD`.

```asm
    .SYNTAX new
    .SYNTAX old
```


### .CONTEXT

Creates a new local context and pushes it to the Context Stack. It is 
forbidden to redefine symbols (with the `EQU` pseudo opcode) or labels during
assembly in the same context.

By creating a new context, it is possible to create local symbols and labels, which
can hold new values, or be completely new. These values will be forgotten when
the context is popped out of the context stack.

When searching for a symbol value or label, the solver searches for the topmost
context and if not found, walks the stack toward the root until it either find
the symbol or label, or fails and report an error.

The two valid arguments are `PUSH` or `POP`.

This command is mainly an exposition of the internal way macros are working.

```asm
    .CONTEXT push
    .CONTEXT pop
```


### .IF/.ELSE/.ENDIF

The conditional commands allow to conditionally assemble parts of the input file.
What's in argument of the `.if` command is evaluated. If it's `0`, then the
part between `.if` and `.endif` or `.else`, whatever comes first, will be skipped. 
In the other cas, the part will be assembled.

If there's an `.else` command, then the part between `.else` and `.endif` will
be assembled with the opposite condition passed to the `.if` argument.

```asm
    .IF 0
    ; This will be skipped
    .ELSE
    ; This will be parsed and assembled
    .ENDIF
```

An `.if` command creates and pushes a local context. It allows to define symbols
and labels local to the conditional code.

```asm
test:   EQU 32
        .IF 1
test:   EQU 42  ; This redefinition is valid        
        .ENDIF
        ; 'test' has again 32 for decimal value.
```

### .MACRO/.ENDMACRO

Macros can be defined to inject common code into assembly when they are called.
A macro definition is started by the `.macro` command and must be associated
to a label, which gives its name to the defined macro.  The definition is ended
by the `.endmacro` command.

A defined macro becomes a command, and it called by its name with a dot (`.`)
prefix. When called, a macro creates and pushes a local context, which allows
symbols and labels to be used locally in the macro.

```asm
inc_hl: .MACRO
        .SYNTAX OLD ; By specfying a syntax inside the macro, it can be called
                    ; wathever the calling code syntax is.
        INL
        JFZ skip
skip:               ; Skip is local to the macro.
        .ENDMACRO
        
        .INC_HL     ; HL is incremented
        .INC_HL     ; ... twice
```

Macros can have parameters, which are specified as a list of names after the
`.macro` command. These names form a substitution dictionary, they are not
symbols in the sense of the solver. If a parameter in the code parsed during
macro extension is exactly a substitution name, then it is replaced by whatever
was present at call site in the matching argument.

```asm
LD_IMM: .MACRO  r1,r2,imm       ; r1, r2 and imm form a substitution dictionnary
        .SYNTAX new
value:  EQU     imm             ; 'imm' is turned into a local symbol named `value` 
        MVI     r1,\HB\value    ; r1 is substituted by the calling argument 
        MVI     r2,\LB\value    ; r2 is substituted by the calling argument
        .ENDMACRO

        .LD_IMM H,L,0x1234      ; r1 will be replaced by H
                                ; r2 will be replaced by L
                                ; imm will be replaced by 0x1234
```

## Arithmetic solver

The assembler has two different arithmetic solvers for expressions.

### Legacy solver

The legacy one mimics the original `as8` solver, which was designed with only
two operands combined by one binary operator. It evaluates the expression from
left to right without any operator precedence.

It understands:

- symbols and labels (which are symbols after all),
- decimal values (e.g.: `1234`),
- binary values suffixed by `b` (e.g.: `10110b`),
- octal values suffixed by `o` (e.g.: `111o`) or, with the `-octal` flag, raw
numbers of three digits (e.g.: `111`),
- hexadecimal values in both suffix form (e.g.: `A0h`) or prefix form (e.g.: `0xA0`),
- single quoted characters (e.g.: `'\n'`), which are replaced by their ASCII value;
it lacks the capability of doing arithmetic with it though,
- five operators: `+`, `-`, `*`, `/` and `#` (which means 'multiply by 256' and
allow to write composed octal, like `100#010`),
- the `\HB\` and `\LB\` prefixes, or the `H(...)` and `L(...)` functions, that
respectively yield the high and low byte of the associated value (see the `.MACRO`
example above),

This solver is chosen when the `-as8` flag is passed on the command line.

### New solver

The new, or default, solver, honors operation precedence and is less surprising
in its results for more complex expressions.

It understands:

- symbols and labels,
- decimal values (e.g.: `1234`),
- binary values suffixed by `b` (e.g.: `10110b`),
- octal values suffixed by `o` (e.g.: `111o`) or, with the `-octal` flag, raw
  numbers of three digits (e.g.: `111`),
- hexadecimal values, only in the prefix form (e.g.: `0xA0`),
- single quoted characters (e.g.: `'\n'`), which are replaced by their ASCII value;
  it HAS the capability of doing arithmetic with it,
- four operators: `+`, `-`, `*`, `/`,
- the `\HB\` and `\LB\` prefixes, or the `H(...)` and `L(...)` functions, that
  respectively yield the high and low byte of the associated value (see the `.MACRO`
  example above),
- the `square()` function as a sample on how to add functions to it in the source code...
