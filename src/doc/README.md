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

### RST

The `RST` opcode parsing understands both addressing forms. It accept both
a number from `0` to `7` or an actual restart address (`0x00`, `0x08`,... `0x38`).

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

### EQU

Provides equivalence between a label and the value in parameter of `EQU`.

`EQU` must be used in a line where a label was specified. The name of this
label then becomes a symbol for the rest of the assembly. It can be use in
any arithmetic expression.

### END

`END` states the end of the input file. It is completely ignored.

### CPU

`CPU` specified the kind of cpu. It must be either `8008` or `i8008`. It
is provided only for compatibility. The assembler doesn't know any other
CPU type.

### DATA/DB

`DATA` and `DB` are equivalent pseudo opcodes that must be followed by
a comma separated list of data that will be converted to successive bytes.

The data can any expression understood by the solver (see further), or
character strings enclosed by double quotes and containing ASCII characters.

### .INCLUDE

Followed by a filename, the `.include` command will insert the content of the
specified file in place.

Included files can themselves include other files.

### .SYNTAX

The command specifies the syntax used by the assembler starting the next line.
The two valid arguments are `NEW` or `OLD`.

### .CONTEXT

Creates a new local context and pushes it to the Context Stack. It is 
forbidden to redefine symbols (with the `EQU` pseudo opcode) or labels during
assembly in the same context.

By creating a new context, it is possible to create local symbols and labels, which
can hold new values, or be completely new. These values will be forgotten when
the context is popped out of the context stack.

The two valid arguments are `PUSH` or `POP`.

This command in mainly an exposition of the internal way macros are working.

### .IF/.ELSE/.ENDIF


### .MACRO/.ENDMACRO


## Arithmetic solver

