#include "files.h"
#include "line_tokenizer.h"
#include "options.h"
#include "symbol_table.h"
#include "utils.h"
#include "opcodes.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>

Options global_options;

int evaluate_argument(const SymbolTable& symbol_table, int line_count, std::string_view arg)
{
    int i, n, j, k;
    char part[4][20], operation[3], extra[80];
    int sum, val;
    int value[4];

    for (i = 0; i < 4; i++)
    {
        part[i][0] = 0;
    }
    for (i = 0; i < 3; i++)
    {
        operation[i] = 0;
    }
    extra[0] = 0;

    if (strncmp(arg.data(), "\\HB\\", 4) == 0)
    {
        i = evaluate_argument(symbol_table, line_count, arg.data() + 4);
        return ((i >> 8) & 0xFF);
    }
    if (strncmp(arg.data(), "\\LB\\", 4) == 0)
    {
        i = evaluate_argument(symbol_table, line_count, arg.data() + 4);
        return (i & 0xFF);
    }

    if (global_options.debug)
    {
        printf("evaluating %s\n", arg.data());
    }
    n = sscanf(arg.data(), "%[^+-/*#]%[+-/*#]%[^+-/*#]%[+-/*#]%[^+-/*#]%[+-/*#]%[^+-/*#]%s",
               part[0], &operation[0], part[1], &operation[1], part[2], &operation[2], part[3],
               extra);
    if (global_options.debug)
    {
        printf("n=%d part0=%s operation0=%c part1=%s operation1=%c part2=%s operation2=%c part3=%s "
               "extra=%s\n",
               n, part[0], operation[0], part[1], operation[1], part[2], operation[2], part[3],
               extra);
    }

    if ((n % 2) == 0)
    {
        fprintf(stderr, "line %d can't evaluate last arg in %s\n", line_count, arg);
        exit(-1);
    }

    n = (n + 1) / 2;
    sum = 0;
    for (j = 0; j < n; j++)
    {

        if (isalpha(part[j][0]))
        {
            if (auto symbol_value = symbol_table.get_symbol_value(part[j]);
                std::get<0>(symbol_value))
            {
                val = std::get<1>(symbol_value);
            }
            else
            {
                fprintf(stderr, "can't find symbol %s\n", part[j]);
                exit(-1);
            }
        }
        else if (tolower(part[j][strlen(part[j]) - 1]) == 'o')
        {
            if (sscanf(part[j], "%o", &val) != 1)
            {
                fprintf(stderr, "error: tried to read octal \"%s\"\n", part[j]);
                exit(-1);
            }
        }
        else if (tolower(part[j][strlen(part[j]) - 1]) == 'h')
        {
            if (sscanf(part[j], "%x", &val) != 1)
            {
                fprintf(stderr, "error: tried to read hex \"%s\"\n", part[j]);
                exit(-1);
            }
        }
        else if ((tolower(part[j][1]) == 'x') && (part[j][0] == '0'))
        {
            if (sscanf(part[j] + 2, "%x", &val) != 1)
            {
                fprintf(stderr, "error: tried to read hex \"%s\"\n", part[j]);
                exit(-1);
            }
        }
        else if (tolower(part[j][strlen(part[j]) - 1]) == 'b')
        {
            i = 0;
            for (k = 0; k < strlen(part[j]) - 1; k++)
            {
                if (part[j][k] == '0')
                    i = i * 2;
                else if (part[j][k] == '1')
                    i = i * 2 + 1;
                else
                {
                    fprintf(stderr, "error: tried to read binary \"%s\"\n", part[j]);
                    exit(-1);
                }
            }
            val = i;
        }
        else
        {
            if ((strlen(part[j]) == 3) && (global_options.input_num_as_octal))
            {
                if (sscanf(part[j], "%o", &val) != 1)
                {
                    fprintf(stderr, "can't evaluate %s as an octal number\n", part[j]);
                    exit(-1);
                }
            }
            else
            {
                if (sscanf(part[j], "%d", &val) != 1)
                {
                    fprintf(stderr, "can't evaluate %s as a decimal number\n", part[j]);
                    exit(-1);
                }
            }
        }
        if (global_options.debug)
            printf("      for %s got value %d\n", part[j], val);
        if (j == 0)
            sum = val;
        else
        {
            if (operation[j - 1] == '+')
                sum = sum + val;
            else if (operation[j - 1] == '-')
                sum = sum - val;
            else if (operation[j - 1] == '*')
                sum = sum * val;
            else if (operation[j - 1] == '/')
                sum = sum / val;
            else if (operation[j - 1] == '#')
                sum = sum * 256 + val;
            else
            {
                fprintf(stderr, "in line %d unknown operation '%c'\n", line_count,
                        operation[j - 1]);
                exit(-1);
            }
        }
        if (global_options.debug)
            printf("     for got sum %d\n", sum);
    }
    return (sum);
}

/* Here are the ways to specify a number or constant: */
/* 0xFF (hex) 010101b (binary) 120 (decimal unless -octal flag, then octal) */
/* 3-digit numeric numbers are either octal or decimal, based on flag */
/* 'w' (character) 123h (hex) 120o (octal)         */

#define MAXONLINE 16

void writebyte(int data, int address, FILE* ofp)
{
    static int currentline[32];
    static int lineaddress;
    static int counter;
    static int first = 1;
    static int oldaddress;
    static unsigned char* progmemory;
    int checksum, i;

    if (address >= (1024 * 16))
    {
        fprintf(stderr, "address of data > %d\n", 1024 * 16 - 1);
        exit(-1);
    }

    if (global_options.generate_binary_file)
    {
        if (first)
        {
            if ((progmemory = static_cast<unsigned char*>(malloc(16384))) == nullptr)
            {
                fprintf(stderr, "Can't allocate 16384 bytes for prog memory\n");
                exit(-1);
            }
            first = 0;
        }
        if (address < 0)
        {
            /* write all output array */
            fwrite(progmemory, 16384, 1, ofp);
        }
        else
            progmemory[address] = (char) (data & 0xFF);
        return;
    }

    /* not binary out, Intel HEX format */

    if (first)
    {
        counter = 0;
        lineaddress = address;
        currentline[counter++] = data;
        first = 0;
    }
    else
    {
        /* if jump in address, or line full, or end of data, write line */
        if ((address != (oldaddress + 1)) || (counter == MAXONLINE) || (address == -1))
        {
            /* write old buffer, then start new line */
            fprintf(ofp, ":%02X%04X%02X", counter, lineaddress, 0);
            checksum = counter + (lineaddress & 0xFF) + ((lineaddress >> 8) & 0xFF);
            for (i = 0; i < counter; i++)
            {
                checksum += currentline[i];
                fprintf(ofp, "%02X", currentline[i]);
            }
            checksum = 0x100 - (checksum & 0xFF);
            fprintf(ofp, "%02X\n", checksum);
            lineaddress = address;
            counter = 0;
            currentline[counter++] = data;
        }
        else
        {
            currentline[counter++] = data;
        }
    }
    oldaddress = address;
    /* if address is -1, signals that end-of-file signal */
    /* should be written (old line written above)  */
    if (address == -1)
    {
        /* write end of file record */
        fprintf(ofp, ":00000001FF\n");
    }
}

int finddata(const SymbolTable& symbol_table, int line_count, const char* line, int* outdata)
{
    const char* ptr;
    char c;
    char cleanline[80];
    char arg[13][20];
    int i, n, *outptr;

    outptr = outdata;
    for (i = 0; i < (strlen(line) - 5); i++)
    {
        if (tolower(line[i]) != 'd')
            continue;
        if (tolower(line[i + 1]) != 'a')
            continue;
        if (tolower(line[i + 2]) != 't')
            continue;
        if (tolower(line[i + 3]) != 'a')
            continue;
        break;
    }
    if (i == (strlen(line) - 5))
    {
        fprintf(stderr, "can't find data code?  Unexpected bug\n");
        exit(-1);
    }
    ptr = &line[i + 4];
    /* move ahead to non-white space */
    for (i = 0; i < strlen(ptr); i++)
        if (!isspace(ptr[i]))
            break;
    ptr = ptr + i;
    if (*ptr == '*')
    {
        if (sscanf(ptr + 1, "%d", &n) != 1)
        {
            fprintf(stderr, " in line %s can't read number to reserve\n", line);
            exit(-1);
        }
        return (0 - n);
    }
    if ((*ptr == '\'') || (*ptr == '"'))
    {
        /* data statement has a quoted string */
        ptr++;
        n = 0;
        while ((*ptr != '"') && (*ptr != '\''))
        {
            if (*ptr == '\\')
            {
                /* escape sequence */
                ptr++;
                if (*ptr == '\\')
                {
                    n++;
                    *(outptr++) = '\\';
                }
                else if (*ptr == 'n')
                {
                    n++;
                    *(outptr++) = '\n';
                }
                else if (*ptr == 't')
                {
                    n++;
                    *(outptr++) = '\t';
                }
                else if (*ptr == '0')
                {
                    n++;
                    *(outptr++) = 0;
                }
                else
                {
                    fprintf(stderr, " in line %d %s unknown escape sequence \\%c\n", line_count,
                            line, *ptr);
                    exit(-1);
                }
            }
            else
            {
                n++;
                *(outptr++) = *ptr;
            }
            ptr++;
            if (strlen(ptr) < 1)
                break;
        }
        /* if "global_options.markascii" option, set the highest bit of these ascii bytes */
        if (global_options.mark_8_ascii)
            for (i = 0; i < n; i++)
                outdata[i] = outdata[i] | 0x80;
    }
    else
    {
        /* data statement has list of arguments to evaluate */

        /* not we're removing comma from original line, list file won't have it */
        for (i = 0; i < (strlen(ptr) + 1); i++)
        {
            c = ptr[i];
            if (c == ';')
                c = 0;
            if (c == ',')
                c = ' ';
            cleanline[i] = c;
        }

        n = sscanf(cleanline, "%s %s %s %s %s %s %s %s %s %s %s %s %s", arg[0], arg[1], arg[2],
                   arg[3], arg[4], arg[5], arg[6], arg[7], arg[8], arg[9], arg[10], arg[11],
                   arg[12]);
        if (n > 12)
        {
            fprintf(stderr, " in line %d %s max length is 12 bytes.\n", line_count, line);
            fprintf(stderr, " Use a second line for more data\n");
            exit(-1);
        }
        for (i = 0; i < n; i++)
        {
            *(outdata++) = evaluate_argument(symbol_table, line_count, arg[i]);
        }
    }
    return (n);
}

void write_listing_header(FILE* lfp)
{
    std::time_t result = std::time(nullptr);
    std::string compile_time{std::asctime(std::localtime(&result))};

    fprintf(lfp, "AS8 assembler for intel 8008, t.e.jones Version 1.0\n");
    fprintf(lfp, "Options: listfile=%d debug=%d ", global_options.generate_list_file,
            global_options.debug);
    fprintf(lfp, "binaryout=%d singlelist=%d\n", global_options.generate_binary_file,
            global_options.single_byte_list);
    fprintf(lfp, "octalnums=%d markascii=%d\n", global_options.input_num_as_octal,
            global_options.mark_8_ascii);
    fprintf(lfp, "Infile=%s\n", global_options.input_filename.c_str());
    fprintf(lfp, "Assembly Performed: %s\n\n", compile_time.c_str());
    if (global_options.single_byte_list)
    {
        fprintf(lfp, "Line Addr.  DAT Source Line\n");
        fprintf(lfp, "---- ------ --- ----------------------------------\n");
    }
    else
    {
        fprintf(lfp, "Line Addr.  CodeBytes   Source Line\n");
        fprintf(lfp, "---- ------ ----------- ----------------------------------\n");
    }
}

void stream_rewind(std::fstream& fstream)
{
    fstream.clear();
    fstream.seekg(std::ios::beg);
}

void first_pass(SymbolTable& symbol_table, Files& files)
{
    /* In the first pass, we just parse through lines to build a symbol table */

    if (global_options.debug || global_options.verbose)
    {
        printf("Pass number One:  Read and Define Symbols\n");
    }

    write_listing_header(files.lfp);

    int line_count = 0;

    int current_address = 0;
    int line_address = 0;
    for (std::string input_line; std::getline(files.input_stream, input_line);)
    {
        line_address = current_address;

        line_count++;
        if (global_options.verbose || global_options.debug)
        {
            std::cout << "     0x" << std::hex << std::uppercase << current_address << " ";
            std::cout << "\"" << input_line << "\"\n";
        }
        /* this function breaks line into separate parts */
        LineTokenizer tokens = parse_line(global_options, input_line, line_count);

        if (global_options.debug)
        {
            tokens.debug_print();
        }

        if (!tokens.label.empty())
        {
            /* Check if the label was already defined. */
            if (auto symbol_value = symbol_table.get_symbol_value(tokens.label);
                std::get<0>(symbol_value))
            {
                std::cerr << " in line " << line_count << " " << input_line;
                std::cerr << " label " << tokens.label;
                std::cerr << " already defined as " << std::get<1>(symbol_value) << "\n";
                exit(-1);
            }

            /* Or define it. */
            int val;
            if (ci_equals(tokens.opcode, "equ") || ci_equals(tokens.opcode, "org"))
            {
                val = evaluate_argument(symbol_table, line_count, tokens.arg1);
            }
            else
            {
                val = current_address;
            }

            if (global_options.debug)
            {
                std::cout << "at address=" << current_address;
                std::cout << std::hex << std::uppercase << "=" << current_address;
                std::cout << " defining " << tokens.label << " = " << std::dec << val;
                std::cout << " =0x" << std::hex << std::uppercase << val << "\n";
            }

            symbol_table.define_symbol(tokens.label, val);
        }

        if (tokens.opcode.empty() || ci_equals(tokens.opcode, "equ") ||
            ci_equals(tokens.opcode, "end"))
        {
            continue;
        }

        if (ci_equals(tokens.opcode, "cpu"))
        {
            if ((!ci_equals(tokens.arg1, "8008")) && (!ci_equals(tokens.arg1, "i8008")))
            {
                std::cerr << " in line " << line_count << " " << input_line;
                std::cerr << " cpu only allowed is \"8008\" or \"i8008\"\n";
                exit(-1);
            }
            continue;
        }

        if (ci_equals(tokens.opcode, "org"))
        {
            if ((current_address = evaluate_argument(symbol_table, line_count, tokens.arg1)) == -1)
            {
                std::cerr << " in line " << line_count << " " << input_line;
                std::cerr << " can't evaluate argument " << tokens.arg1 << "\n";
                exit(-1);
            }
        }
        else if (ci_equals(tokens.opcode, "data"))
        {
            int data_list[80];
            int n = finddata(symbol_table, line_count, input_line.c_str(), data_list);
            if (global_options.debug)
            {
                printf("got %d items in data list\n", n);
            }
            /* a negative number denotes that much space to save, but not specifying data */
            /* if so, just change sign to positive */
            if (n < 0)
                n = 0 - n;
            current_address += n;
            continue;
        }
        else if (auto [found, opcode] = find_opcode(tokens.opcode); found)
        {
            /* found the opcode */
            if (opcode.rule == 0)
            {
                current_address += 1;
            }
            else if (opcode.rule == 1)
            {
                current_address += 2;
            }
            else if (opcode.rule == 2)
            {
                current_address += 3;
            }
            else if (opcode.rule == 3)
            {
                current_address += 1;
            }
        }
        else
        {
            std::cerr << " in line " << line_count << " " << input_line;
            std::cerr << " undefined opcode " << tokens.opcode << "\n";
            exit(-1);
        }
    }
}

void second_pass(const SymbolTable& symbol_table, Files& files)
{
    /* Symbols are defined. Second pass. */
    int arg1;

    if (global_options.verbose || global_options.debug)
    {
        std::cout << "Pass number Two:  Re-read and assemble codes\n";
    }

    char single_space_pad[9]; /* this is some extra padding if we use single space list file */
    if (global_options.single_byte_list)
    {
        single_space_pad[0] = 0;
    }

    else
    {
        strcpy(single_space_pad, "        ");
    }

    int line_count = 0;
    int current_address = 0;
    int line_address = 0;

    stream_rewind(files.input_stream);
    for (std::string input_line; std::getline(files.input_stream, input_line);)
    {
        line_address = current_address;
        line_count++;

        if (global_options.verbose || global_options.debug)
        {
            printf("     0x%X \"%s\"\n", current_address, input_line.c_str());
        }

        LineTokenizer tokens = parse_line(global_options, input_line, line_count);
        int args = tokens.arg_count;

        if (tokens.opcode.empty())
        {
            /* Must just be a comment line (or label only) */
            if (global_options.generate_list_file)
            {
                fprintf(files.lfp, "%4d            %s%s\n", line_count, single_space_pad,
                        input_line.c_str());
            }
            continue;
        }

        /* Check if this opcode is one of the pseudo ops. */
        if (ci_equals(tokens.opcode, "equ"))
        {
            if (global_options.generate_list_file)
                fprintf(files.lfp, "%4d            %s%s\n", line_count, single_space_pad,
                        input_line.c_str());
            continue;
        }
        if (ci_equals(tokens.opcode, "cpu"))
        {
            if (global_options.generate_list_file)
                fprintf(files.lfp, "%4d            %s%s\n", line_count, single_space_pad,
                        input_line.c_str());
            continue;
        }

        if (ci_equals(tokens.opcode, "org"))
        {
            if ((current_address = evaluate_argument(symbol_table, line_count, tokens.arg1)) == -1)
            {
                fprintf(stderr, " in input_line.c_str() %d %s can't evaluate argument %s\n",
                        line_count, input_line.c_str(), tokens.arg1.c_str());
                exit(-1);
            }
            if (global_options.generate_list_file)
                fprintf(files.lfp, "%4d            %s%s\n", line_count, single_space_pad,
                        input_line.c_str());
            continue;
        }
        if (ci_equals(tokens.opcode, "end"))
        {
            if (global_options.generate_list_file)
                fprintf(files.lfp, "%4d            %s%s\n", line_count, single_space_pad,
                        input_line.c_str());
            /* could break here, but rather than break, */
            /* we will go ahead and check for more with a continue */
            continue;
        }
        else if (ci_equals(tokens.opcode, "data"))
        {
            int data_list[80];
            int n = finddata(symbol_table, line_count, input_line.c_str(), data_list);
            /* if n is negative, that number of bytes are just reserved */
            if (n < 0)
            {
                if (global_options.generate_list_file)
                    fprintf(files.lfp, "%4d %02o-%03o     %s%s\n", line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF), single_space_pad,
                            input_line.c_str());
                current_address += 0 - n;
                continue;
            }
            for (int i = 0; i < n; i++)
            {
                writebyte(data_list[i], current_address++, files.ofp);
            }
            if (global_options.generate_list_file)
            {
                if (global_options.single_byte_list)
                {
                    fprintf(files.lfp, "%4d %02o-%03o %03o %s\n", line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF), data_list[0],
                            input_line.c_str());
                    for (int i = 1; i < n; i++)
                    {
                        fprintf(files.lfp, "%     %02o-%03o %03o\n", line_count,
                                (((line_address + i) >> 8) & 0xFF), ((line_address + i) & 0xFF),
                                data_list[i]);
                    }
                }
                else
                {
                    fprintf(files.lfp, "%4d %02o-%03o ", line_count, ((line_address >> 8) & 0xFF),
                            (line_address & 0xFF));
                    if (n == 1)
                        fprintf(files.lfp, "%03o          %s\n", data_list[0], input_line.c_str());
                    else if (n == 2)
                        fprintf(files.lfp, "%03o %03o      %s\n", data_list[0], data_list[1],
                                input_line.c_str());
                    else if (n > 2)
                    {
                        fprintf(files.lfp, "%03o %03o %03o  %s\n", data_list[0], data_list[1],
                                data_list[2], input_line.c_str());
                        int* ptr = data_list + 3;
                        n -= 3;
                        line_address += 3;
                        while (n > 0)
                        {
                            /*	    fprintf(files.lfp,"            "); */
                            fprintf(files.lfp, "     %02o-%03o ", ((line_address >> 8) & 0xFF),
                                    (line_address & 0xFF));
                            if (n > 2)
                            {
                                fprintf(files.lfp, "%03o %03o %03o\n", ptr[0], ptr[1], ptr[2]);
                                ptr += 3;
                                n -= 3;
                                line_address += 3;
                            }
                            else
                            {
                                for (int i = 0; i < n; i++)
                                {
                                    fprintf(files.lfp, "%03o ", ptr[0]);
                                    ptr++;
                                }
                                n = 0;
                                fprintf(files.lfp, "\n");
                            }
                        }
                    }
                }
            }
            continue;
        }
        /* Now we should have an opcode. */

        auto [found, opcode] = find_opcode(tokens.opcode);
        if (!found)
        {
            fprintf(stderr, " in line %d %s undefined opcode %s\n", line_count, input_line.c_str(),
                    tokens.opcode.c_str());
            exit(-1);
        }
        /* found the opcode */
        /* check that we have right number of arguments */
        if (((opcode.rule == 0) && (args != 0)) || ((opcode.rule == 1) && (args != 1)) ||
            ((opcode.rule == 2) && (args != 1)) || ((opcode.rule == 3) && (args != 1)))
        {
            fprintf(stderr, " in line %d %s we see an unexpected %d arguments\n", line_count,
                    input_line.c_str(), args);
            exit(-1);
        }
        if (args == 1)
        {
            if ((arg1 = evaluate_argument(symbol_table, line_count, tokens.arg1)) == -1)
            {
                fprintf(stderr, " in line %d %s can't evaluate argument %s\n", line_count,
                        input_line.c_str(), tokens.arg1.c_str());
                exit(-1);
            }
        }
        /*
     *
     * Now, each opcode, is categorized into different
     * "rules" which states how arguments are combined
     * with opcode to get machine codes.
     *
     */

        if (opcode.rule == 0)
        {
            /* single byte, no arguments */
            writebyte(opcode.code, current_address++, files.ofp);
            if (global_options.generate_list_file)
                fprintf(files.lfp, "%4d %02o-%03o %03o %s%s\n", line_count,
                        ((line_address >> 8) & 0xFF), (line_address & 0xFF), opcode.code,
                        single_space_pad, input_line.c_str());
        }
        else if (opcode.rule == 1)
        {
            /* single byte, must follow */
            if ((arg1 > 255) || (arg1 < 0))
            {
                fprintf(stderr, " in line %d %s expected argument 0-255\n", line_count,
                        input_line.c_str());
                fprintf(stderr, "    instead got %s=%d\n", tokens.arg1.c_str(), arg1);
                exit(-1);
            }
            int code = opcode.code;
            writebyte(code, current_address++, files.ofp);
            writebyte(arg1, current_address++, files.ofp);
            if (global_options.generate_list_file)
            {
                if (global_options.single_byte_list)
                {
                    fprintf(files.lfp, "%4d %02o-%03o %03o %s\n", line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF), code,
                            input_line.c_str());
                    line_address++;
                    fprintf(files.lfp, "     %02o-%03o %03o\n", (((line_address) >> 8) & 0xFF),
                            ((line_address) &0xFF), arg1);
                }
                else
                {
                    fprintf(files.lfp, "%4d %02o-%03o %03o %03o     %s\n", line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF), code, arg1,
                            input_line.c_str());
                }
            }
        }
        else if (opcode.rule == 2)
        {
            /* two byte address to follow */
            if ((arg1 > 1024 * 16) || (arg1 < 0))
            {
                fprintf(stderr, " in input_line.c_str() %d %s expected argument 0-%d\n", line_count,
                        input_line.c_str(), 1024 * 16);
                fprintf(stderr, "    instead got %s=%d\n", tokens.arg1.c_str(), arg1);
                exit(-1);
            }
            int code = opcode.code;
            int lowbyte = (0xFF & arg1);
            int highbyte = (0xFF & (arg1 >> 8));
            writebyte(code, current_address++, files.ofp);
            writebyte(lowbyte, current_address++, files.ofp);
            writebyte(highbyte, current_address++, files.ofp);
            if (global_options.generate_list_file)
            {
                if (global_options.single_byte_list)
                {
                    fprintf(files.lfp, "%4d %02o-%03o %03o %s\n", line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF), code,
                            input_line.c_str());
                    line_address++;
                    fprintf(files.lfp, "     %02o-%03o %03o\n", ((line_address >> 8) & 0xFF),
                            (line_address & 0xFF), lowbyte);
                    line_address++;
                    fprintf(files.lfp, "     %02o-%03o %03o\n", ((line_address >> 8) & 0xFF),
                            (line_address & 0xFF), highbyte);
                }
                else
                {
                    fprintf(files.lfp, "%4d %02o-%03o %03o %03o %03o %s\n", line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF), code, lowbyte,
                            highbyte, input_line.c_str());
                }
            }
        }
        else if (opcode.rule == 3)
        {
            /* have an input or output instruction */
            int maxport;

            if (opcode.mnemonic[0] == 'i')
                maxport = 7;
            else
                maxport = 23;
            if ((arg1 > maxport) || (arg1 < 0))
            {
                fprintf(stderr, " in input_line.c_str() %d %s expected port 0-%d\n", line_count,
                        input_line.c_str(), maxport);
                fprintf(stderr, "    instead got %s=%d\n", tokens.arg1.c_str(), arg1);
                exit(-1);
            }
            int code = opcode.code + (arg1 << 1);
            writebyte(code, current_address++, files.ofp);
            if (global_options.generate_list_file)
                fprintf(files.lfp, "%4d %02o-%03o %03o %s%s\n", line_count,
                        ((line_address >> 8) & 0xFF), (line_address & 0xFF), code, single_space_pad,
                        input_line.c_str());
        }
        else
        {
            fprintf(stderr, " in input_line.c_str() %d %s can't comprehend rule %d\n", line_count,
                    input_line.c_str(), opcode.rule);
            exit(-1);
        }
    }
}

int main(int argc, const char** argv)
{
    try
    {
        global_options.parse(argc, argv);
    }
    catch (InvalidCommandLine&)
    {
        exit(-1);
    }

    Files files(global_options);
    SymbolTable symbol_table;

    first_pass(symbol_table, files);
    second_pass(symbol_table, files);

    writebyte(-1, -1, files.ofp); /* signal to close off output file */

    /* write symbol table to listfile */
    if (global_options.generate_list_file)
    {
        symbol_table.list_symbols(files.lfp);
    }
}
