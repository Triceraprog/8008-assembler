#include "files.h"
#include "line_tokenizer.h"
#include "opcodes.h"
#include "options.h"
#include "symbol_table.h"
#include "utils.h"

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <regex>

Options global_options;

/*
 * Preparing a context object
struct ParsingContext
{
    SymbolTable & symbol_table;
    const Options & options;
    std::string_view line;
    int current_line_count;
    // Logger logger

};
 */

int parse_number_value(const std::string& to_parse, int base, std::string_view type_name)
{
    int val;
    try
    {
        val = std::stoi(to_parse, nullptr, base);
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << "error: tried to read " << type_name << " " << to_parse
                  << ". Argument is invalid.\n";
        exit(-1);
    }
    catch (const std::out_of_range& e)
    {
        std::cerr << "error: tried to read " << type_name << " " << to_parse
                  << ". Argument is out of range.\n";
        exit(-1);
    }
    return val;
}

namespace
{
    std::regex data_rule{"[dD][aA][tT][aA]\\s*"};
    std::regex number_scan{R"((0x)?[[:xdigit:]]+)"};
    std::regex not_an_operator{R"(^\s*([^\+\*-/#\s]+)\s*)"};
    std::regex except_comma{R"(([^,\s]*))"};
}

int evaluate_argument(const SymbolTable& symbol_table, int current_line_count, std::string_view arg)
{
    int i, n, j, k;
    char part[4][20], operation[3], extra[80];
    int sum, val;

    for (i = 0; i < 4; i++)
    {
        part[i][0] = 0;
    }
    for (i = 0; i < 3; i++)
    {
        operation[i] = 0;
    }
    extra[0] = 0;

    if (arg.starts_with("\\HB\\"))
    {
        int value = evaluate_argument(symbol_table, current_line_count, arg.begin() + 4);
        return ((value >> 8) & 0xFF);
    }
    if (arg.starts_with("\\LB\\"))
    {
        int value = evaluate_argument(symbol_table, current_line_count, arg.begin() + 4);
        return (value & 0xFF);
    }

    if (global_options.debug)
    {
        std::cout << "evaluating " << arg << "\n";
    }
    int value[4];

    std::string to_parse(arg);
    std::vector<std::string> args;
    std::vector<std::string> operations;
    std::string_view arg_to_parse{arg};

    while (!arg_to_parse.empty())
    {
        std::string value_to_parse(arg_to_parse);
        std::smatch data_match;
        bool found = std::regex_search(value_to_parse, data_match, not_an_operator);

        if (!found)
        {
            std::cerr << "Error, expected value, found: " << arg_to_parse << std::endl;
            exit(-1);
            break;
        }

        arg_to_parse = arg_to_parse.substr(data_match.length());

        auto trimmed_data = std::string_view{data_match.str()};
        trimmed_data = trimmed_data.substr(trimmed_data.find_first_not_of(" \t"));
        trimmed_data = trimmed_data.substr(0, trimmed_data.find_last_not_of(" \t") + 1);

        //std::cout << "Trimmed" << trimmed_data << std::endl;

        args.emplace_back(trimmed_data);

        if (!arg_to_parse.empty())
        {
            char op = arg_to_parse.front();

            switch (op)
            {
                case '+':
                case '-':
                case '/':
                case '*':
                case '#':
                    operations.push_back(std::string{op});
                    arg_to_parse = arg_to_parse.substr(1);
                    break;
                default:
                    std::cerr << "Error '" << op << "'" << std::endl;
                    exit(-1);
                    break;
            }
        }
    }

    n = sscanf(to_parse.c_str(), "%[^+-/*#]%[+-/*#]%[^+-/*#]%[+-/*#]%[^+-/*#]%[+-/*#]%[^+-/*#]%s",
               part[0], &operation[0], part[1], &operation[1], part[2], &operation[2], part[3],
               extra);
    if (global_options.debug)
    {
        printf("n=%d part0=%s operation0=%c part1=%s operation1=%c part2=%s operation2=%c part3=%s "
               "extra=%s\n",
               n, part[0], operation[0], part[1], operation[1], part[2], operation[2], part[3],
               extra);
        std::cout << "parts=" << args.size() << ":";
        std::ranges::copy(args, std::ostream_iterator<std::string>(std::cout, ","));
        std::cout << std::endl;
    }

    /*
    if (args.size() % 2 == 0)
    {
        std::cerr << "line " << current_line_count << " can't evaluate last arg in " << arg << "\n";
    }
     */

    n = (n + 1) / 2;
    sum = 0;
    for (j = 0; j < n; j++)
    {
        if (std::isalpha(args[j][0]))
        {
            if (auto symbol_value = symbol_table.get_symbol_value(args[j]);
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
        else if (tolower(args[j].back()) == 'o')
        {
            val = parse_number_value(args[j], 8, "octal");
        }
        else if (tolower(args[j].back()) == 'h')
        {
            val = parse_number_value(args[j], 16, "hex");
        }
        else if (args[j].starts_with("0x") || args[j].starts_with("0X"))
        {
            val = parse_number_value(args[j].substr(2), 16, "hex");
        }
        else if (tolower(args[j].back() == 'b'))
        {
            val = parse_number_value(args[j], 2, "binary");
        }
        else
        {
            if ((strlen(part[j]) == 3) && (global_options.input_num_as_octal))
            {
                val = parse_number_value(args[j], 8, "octal");
            }
            else
            {
                val = parse_number_value(args[j], 10, "decimal");
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
                fprintf(stderr, "in line %d unknown operation '%c'\n", current_line_count,
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

int find_data(const SymbolTable& symbol_table, int current_line_count, const std::string_view line,
              int* outdata)
{
    std::string line_as_string{line};
    std::smatch data_match;
    bool found = std::regex_search(line_as_string, data_match, data_rule);

    if (!found)
    {
        std::cerr << "can't find data code?  Unexpected bug\n";
        exit(-1);
    }

    auto after_data_position = data_match.position() + data_match.length();
    auto data_part = line.substr(after_data_position);

    if (data_part.empty())
    {
        return 0;
    }

    if (data_part.front() == '*')
    {
        // 'DATA *NNN' reserve NNN bytes.
        try
        {
            int number_to_reserve = std::stoi(data_part.substr(1).data());
            return 0 - number_to_reserve;
        }
        catch (...)
        {
            std::cerr << " in line %s can't read number to reserve\n";
            exit(-1);
        }
    }
    if ((data_part.front() == '\'') || (data_part.front() == '"'))
    {
        // DATA "..." or DATA '...' declares a string of characters
        // Warning: there's a syntax limitation. If a comment contains a quote, then
        // the result will be wrong.
        const char quote_to_find = data_part.front();
        auto last_quote_position = data_part.find_last_of(quote_to_find);
        auto string_content = data_part.substr(1, last_quote_position - 1);

        bool escape_char = false;
        int* out_pointer = outdata;

        for (char char_data : string_content)
        {
            if (escape_char)
            {
                escape_char = false;
                switch (char_data)
                {
                    case '\\':
                        *(out_pointer++) = '\\';
                        break;
                    case 'n':
                        *(out_pointer++) = '\n';
                        break;
                    case 't':
                        *(out_pointer++) = '\t';
                        break;
                    case '0':
                        *(out_pointer++) = '\0';
                        break;
                    default:
                        std::cerr << " in line " << current_line_count << " " << line;
                        std::cerr << " unknown escape sequence \\" << char_data << "\n";
                        exit(-1);
                }
            }
            else if (char_data == '\\')
            {
                escape_char = true;
            }
            else
            {
                *(out_pointer++) = static_cast<u_char>(char_data);
            }
        }

        /* If "markascii" option is set, highest bit of these ascii bytes are forced to 1. */
        if (global_options.mark_8_ascii)
        {
            for (auto* p = outdata; outdata < out_pointer; p++)
            {
                *p |= 0x80;
            }
        }

        return static_cast<int>(out_pointer - outdata);
    }
    else
    {
        /* DATA xxx,xxx,xxx,xxx */
        const auto first_comment = data_part.find_first_of(';');
        auto without_comment = std::string{data_part.substr(0, first_comment)};

        int byte_count = 0;

        //std::cout << "DATA\n";
        //std::cout << without_comment << std::endl;

        auto begin =
                std::sregex_iterator(without_comment.begin(), without_comment.end(), except_comma);
        auto end = std::sregex_iterator();

        for (auto it = begin; it != end; it++)
        {
            std::string sub = it->str();

            if (sub.empty())
            {
                continue;
            }
            //std::cout << sub << std::endl;
            *(outdata++) = evaluate_argument(symbol_table, current_line_count, sub);

            byte_count += 1;

            if (byte_count > 12)
            {
                std::cerr << " in line " << current_line_count << " " << line;
                std::cerr << " max length is 12 bytes.\n";
                std::cerr << " Use a second line for more data\n";
                exit(-1);
            }
        }

        //std::cout << "---" << std::endl;

        return byte_count;
    }
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

    int current_line_count = 0;

    int current_address = 0;
    int line_address = 0;
    for (std::string input_line; std::getline(files.input_stream, input_line);)
    {
        line_address = current_address;

        current_line_count++;
        if (global_options.verbose || global_options.debug)
        {
            std::cout << "     0x" << std::hex << std::uppercase << current_address << " ";
            std::cout << "\"" << input_line << "\"\n";
        }
        /* this function breaks line into separate parts */
        LineTokenizer tokens = parse_line(global_options, input_line, current_line_count);

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
                std::cerr << " in line " << current_line_count << " " << input_line;
                std::cerr << " label " << tokens.label;
                std::cerr << " already defined as " << std::get<1>(symbol_value) << "\n";
                exit(-1);
            }

            /* Or define it. */
            int val;
            if (ci_equals(tokens.opcode, "equ") || ci_equals(tokens.opcode, "org"))
            {
                val = evaluate_argument(symbol_table, current_line_count, tokens.arg1);
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
                std::cerr << " in line " << current_line_count << " " << input_line;
                std::cerr << " cpu only allowed is \"8008\" or \"i8008\"\n";
                exit(-1);
            }
            continue;
        }

        if (ci_equals(tokens.opcode, "org"))
        {
            if ((current_address =
                         evaluate_argument(symbol_table, current_line_count, tokens.arg1)) == -1)
            {
                std::cerr << " in line " << current_line_count << " " << input_line;
                std::cerr << " can't evaluate argument " << tokens.arg1 << "\n";
                exit(-1);
            }
        }
        else if (ci_equals(tokens.opcode, "data"))
        {
            int data_list[80];
            int n = find_data(symbol_table, current_line_count, input_line.c_str(), data_list);
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
            std::cerr << " in line " << current_line_count << " " << input_line;
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

    int current_line_count = 0;
    int current_address = 0;
    int line_address = 0;

    stream_rewind(files.input_stream);
    for (std::string input_line; std::getline(files.input_stream, input_line);)
    {
        line_address = current_address;
        current_line_count++;

        if (global_options.verbose || global_options.debug)
        {
            printf("     0x%X \"%s\"\n", current_address, input_line.c_str());
        }

        LineTokenizer tokens = parse_line(global_options, input_line, current_line_count);
        int args = tokens.arg_count;

        if (tokens.opcode.empty())
        {
            /* Must just be a comment line (or label only) */
            if (global_options.generate_list_file)
            {
                fprintf(files.lfp, "%4d            %s%s\n", current_line_count, single_space_pad,
                        input_line.c_str());
            }
            continue;
        }

        /* Check if this opcode is one of the pseudo ops. */
        if (ci_equals(tokens.opcode, "equ"))
        {
            if (global_options.generate_list_file)
                fprintf(files.lfp, "%4d            %s%s\n", current_line_count, single_space_pad,
                        input_line.c_str());
            continue;
        }
        if (ci_equals(tokens.opcode, "cpu"))
        {
            if (global_options.generate_list_file)
                fprintf(files.lfp, "%4d            %s%s\n", current_line_count, single_space_pad,
                        input_line.c_str());
            continue;
        }

        if (ci_equals(tokens.opcode, "org"))
        {
            if ((current_address =
                         evaluate_argument(symbol_table, current_line_count, tokens.arg1)) == -1)
            {
                fprintf(stderr, " in input_line.c_str() %d %s can't evaluate argument %s\n",
                        current_line_count, input_line.c_str(), tokens.arg1.c_str());
                exit(-1);
            }
            if (global_options.generate_list_file)
                fprintf(files.lfp, "%4d            %s%s\n", current_line_count, single_space_pad,
                        input_line.c_str());
            continue;
        }
        if (ci_equals(tokens.opcode, "end"))
        {
            if (global_options.generate_list_file)
                fprintf(files.lfp, "%4d            %s%s\n", current_line_count, single_space_pad,
                        input_line.c_str());
            /* could break here, but rather than break, */
            /* we will go ahead and check for more with a continue */
            continue;
        }
        else if (ci_equals(tokens.opcode, "data"))
        {
            int data_list[80];
            int n = find_data(symbol_table, current_line_count, input_line.c_str(), data_list);
            /* if n is negative, that number of bytes are just reserved */
            if (n < 0)
            {
                if (global_options.generate_list_file)
                    fprintf(files.lfp, "%4d %02o-%03o     %s%s\n", current_line_count,
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
                    fprintf(files.lfp, "%4d %02o-%03o %03o %s\n", current_line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF), data_list[0],
                            input_line.c_str());
                    for (int i = 1; i < n; i++)
                    {
                        fprintf(files.lfp, "%     %02o-%03o %03o\n", current_line_count,
                                (((line_address + i) >> 8) & 0xFF), ((line_address + i) & 0xFF),
                                data_list[i]);
                    }
                }
                else
                {
                    fprintf(files.lfp, "%4d %02o-%03o ", current_line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF));
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
            fprintf(stderr, " in line %d %s undefined opcode %s\n", current_line_count,
                    input_line.c_str(), tokens.opcode.c_str());
            exit(-1);
        }
        /* found the opcode */
        /* check that we have right number of arguments */
        if (((opcode.rule == 0) && (args != 0)) || ((opcode.rule == 1) && (args != 1)) ||
            ((opcode.rule == 2) && (args != 1)) || ((opcode.rule == 3) && (args != 1)))
        {
            fprintf(stderr, " in line %d %s we see an unexpected %d arguments\n",
                    current_line_count, input_line.c_str(), args);
            exit(-1);
        }
        if (args == 1)
        {
            if ((arg1 = evaluate_argument(symbol_table, current_line_count, tokens.arg1)) == -1)
            {
                fprintf(stderr, " in line %d %s can't evaluate argument %s\n", current_line_count,
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
                fprintf(files.lfp, "%4d %02o-%03o %03o %s%s\n", current_line_count,
                        ((line_address >> 8) & 0xFF), (line_address & 0xFF), opcode.code,
                        single_space_pad, input_line.c_str());
        }
        else if (opcode.rule == 1)
        {
            /* single byte, must follow */
            if ((arg1 > 255) || (arg1 < 0))
            {
                fprintf(stderr, " in line %d %s expected argument 0-255\n", current_line_count,
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
                    fprintf(files.lfp, "%4d %02o-%03o %03o %s\n", current_line_count,
                            ((line_address >> 8) & 0xFF), (line_address & 0xFF), code,
                            input_line.c_str());
                    line_address++;
                    fprintf(files.lfp, "     %02o-%03o %03o\n", (((line_address) >> 8) & 0xFF),
                            ((line_address) &0xFF), arg1);
                }
                else
                {
                    fprintf(files.lfp, "%4d %02o-%03o %03o %03o     %s\n", current_line_count,
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
                fprintf(stderr, " in input_line.c_str() %d %s expected argument 0-%d\n",
                        current_line_count, input_line.c_str(), 1024 * 16);
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
                    fprintf(files.lfp, "%4d %02o-%03o %03o %s\n", current_line_count,
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
                    fprintf(files.lfp, "%4d %02o-%03o %03o %03o %03o %s\n", current_line_count,
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
                fprintf(stderr, " in input_line.c_str() %d %s expected port 0-%d\n",
                        current_line_count, input_line.c_str(), maxport);
                fprintf(stderr, "    instead got %s=%d\n", tokens.arg1.c_str(), arg1);
                exit(-1);
            }
            int code = opcode.code + (arg1 << 1);
            writebyte(code, current_address++, files.ofp);
            if (global_options.generate_list_file)
                fprintf(files.lfp, "%4d %02o-%03o %03o %s%s\n", current_line_count,
                        ((line_address >> 8) & 0xFF), (line_address & 0xFF), code, single_space_pad,
                        input_line.c_str());
        }
        else
        {
            fprintf(stderr, " in input_line.c_str() %d %s can't comprehend rule %d\n",
                    current_line_count, input_line.c_str(), opcode.rule);
            exit(-1);
        }
    }
}
