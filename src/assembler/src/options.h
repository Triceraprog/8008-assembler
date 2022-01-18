#ifndef INC_8008_ASSEMBLER_OPTIONS_H
#define INC_8008_ASSEMBLER_OPTIONS_H

struct Options
{
    int verbose = 0;
    int generate_list_file = 1;
    int debug = 0;
    int single_byte_list = 0;
    int generate_binary_file = 0;
    int input_num_as_octal = 0;
    int mark_8_ascii = 0;
};

#endif //INC_8008_ASSEMBLER_OPTIONS_H
