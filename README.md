# Assembler for the i8008

This is an assembler for the Intel 8008 processor.

It aims to:

  * understand the Micral N specific instructions,
  * support macros,
  * understand the "new opcode" syntax (*MOV A,B* rather than *LAB*),
  * allow Anonymous Local Labels, 
  * retain compatibility with as8.c (see below), and thus Mark 8 specific format.

## Documentation

Documentation is available [here](doc/README.md).

## History

This assembler was created with the objective of supporting the specific instructions for the "Micral N" micro-computer.

It grew out of "as8.c" with permission of the author T.E.Jones. Thank you. "as8.c" is a compact, one C file, assembler
for the 8008 with a "Mark 8" support that will compile simply with a C compiler like gcc. I've been using it a lot.

Compared to it, this assembler is more verbose, needs a C++20 compiler, is a bit slower to process the file and much
slower to compile.

At the start of the developement of this assembler, I wrote a test suite validated by "as8.c", which was
then continuously verified to watch for regression. Most of the code has been rewritten by now, but traces of
the original "as8.c" can be found in the global structure or some error messages.
