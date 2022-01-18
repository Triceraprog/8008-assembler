#include "options.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

/* this is a lot of symbols, but needed for SCELBAL */
#define MAXSYMBOLS 1000

struct
{
    char label[10];
    int value;
} symbols[MAXSYMBOLS];

int numsymbols;

FILE *ifp, *ofp, *lfp;
int linecount;

Options global_options;

struct
{
    char* mnemonic;
    unsigned char code;
    int rule;
} opcodes[] = {
        /* first the basic load immediates */
        "lai", 0006, 1, "lbi", 0016, 1, "lci", 0026, 1, "ldi", 0036, 1, "lei", 0046, 1, "lhi", 0056,
        1, "lli", 0066, 1, "lmi", 0076, 1,
        /* now the increment registers */
        "inb", 0010, 0, "inc", 0020, 0, "ind", 0030, 0, "ine", 0040, 0, "inh", 0050, 0, "inl", 0060,
        0,
        /* now decrement registers */
        "dcb", 0011, 0, "dcc", 0021, 0, "dcd", 0031, 0, "dce", 0041, 0, "dch", 0051, 0, "dcl", 0061,
        0,
        /* next add registers to accum */
        "ada", 0200, 0, "adb", 0201, 0, "adc", 0202, 0, "add", 0203, 0, "ade", 0204, 0, "adh", 0205,
        0, "adl", 0206, 0, "adm", 0207, 0, "adi", 0004, 1, "aca", 0210, 0, "acb", 0211, 0, "acc",
        0212, 0, "acd", 0213, 0, "ace", 0214, 0, "ach", 0215, 0, "acl", 0216, 0, "acm", 0217, 0,
        "aci", 0014, 1,
        /* next subtract registers from accumulator */
        "sua", 0220, 0, "sub", 0221, 0, "suc", 0222, 0, "sud", 0223, 0, "sue", 0224, 0, "suh", 0225,
        0, "sul", 0226, 0, "sum", 0227, 0, "sui", 0024, 1, "sba", 0230, 0, "sbb", 0231, 0, "sbc",
        0232, 0, "sbd", 0233, 0, "sbe", 0234, 0, "sbh", 0235, 0, "sbl", 0236, 0, "sbm", 0237, 0,
        "sbi", 0034, 1,
        /* and registers with accumulator */
        "nda", 0240, 0, "ndb", 0241, 0, "ndc", 0242, 0, "ndd", 0243, 0, "nde", 0244, 0, "ndh", 0245,
        0, "ndl", 0246, 0, "ndm", 0247, 0, "ndi", 0044, 1,
        /* xor registers with accumulator */
        "xra", 0250, 0, "xrb", 0251, 0, "xrc", 0252, 0, "xrd", 0253, 0, "xre", 0254, 0, "xrh", 0255,
        0, "xrl", 0256, 0, "xrm", 0257, 0, "xri", 0054, 1,
        /* or registers with accumulator */
        "ora", 0260, 0, "orb", 0261, 0, "orc", 0262, 0, "ord", 0263, 0, "ore", 0264, 0, "orh", 0265,
        0, "orl", 0266, 0, "orm", 0267, 0, "ori", 0064, 1,
        /* compare registers with accumulator */
        "cpa", 0270, 0, "cpb", 0271, 0, "cpc", 0272, 0, "cpd", 0273, 0, "cpe", 0274, 0, "cph", 0275,
        0, "cpl", 0276, 0, "cpm", 0277, 0, "cpi", 0074, 1,
        /* a halt code */
        "hlt", 0001, 0,
        /* now all the load registers */
        "laa", 0300, 0, "lab", 0301, 0, "lac", 0302, 0, "lad", 0303, 0, "lae", 0304, 0, "lah", 0305,
        0, "lal", 0306, 0, "lam", 0307, 0, "lba", 0310, 0, "lbb", 0311, 0, "lbc", 0312, 0, "lbd",
        0313, 0, "lbe", 0314, 0, "lbh", 0315, 0, "lbl", 0316, 0, "lbm", 0317, 0, "lca", 0320, 0,
        "lcb", 0321, 0, "lcc", 0322, 0, "lcd", 0323, 0, "lce", 0324, 0, "lch", 0325, 0, "lcl", 0326,
        0, "lcm", 0327, 0, "lda", 0330, 0, "ldb", 0331, 0, "ldc", 0332, 0, "ldd", 0333, 0, "lde",
        0334, 0, "ldh", 0335, 0, "ldl", 0336, 0, "ldm", 0337, 0, "lea", 0340, 0, "leb", 0341, 0,
        "lec", 0342, 0, "led", 0343, 0, "lee", 0344, 0, "leh", 0345, 0, "lel", 0346, 0, "lem", 0347,
        0, "lha", 0350, 0, "lhb", 0351, 0, "lhc", 0352, 0, "lhd", 0353, 0, "lhe", 0354, 0, "lhh",
        0355, 0, "lhl", 0356, 0, "lhm", 0357, 0, "lla", 0360, 0, "llb", 0361, 0, "llc", 0362, 0,
        "lld", 0363, 0, "lle", 0364, 0, "llh", 0365, 0, "lll", 0366, 0, "llm", 0367, 0, "lma", 0370,
        0, "lmb", 0371, 0, "lmc", 0372, 0, "lmd", 0373, 0, "lme", 0374, 0, "lmh", 0375, 0, "lml",
        0376, 0,
        /* rotate the accumulator */
        "ral", 0022, 0, "rar", 0032, 0, "rlc", 0002, 0, "rrc", 0012, 0,
        /* jump instructions */
        "jmp", 0104, 2, "jfc", 0100, 2, "jfz", 0110, 2, "jfs", 0120, 2, "jfp", 0130, 2, "jtc", 0140,
        2, "jtz", 0150, 2, "jts", 0160, 2, "jtp", 0170, 2,
        /* call instructions */
        "cal", 0106, 2, "cfc", 0102, 2, "cfz", 0112, 2, "cfs", 0122, 2, "cfp", 0132, 2, "ctc", 0142,
        2, "ctz", 0152, 2, "cts", 0162, 2, "ctp", 0172, 2,
        /* return instructions */
        "ret", 0007, 0, "rfc", 0003, 0, "rfz", 0013, 0, "rfs", 0023, 0, "rfp", 0033, 0, "rtc", 0043,
        0, "rtz", 0053, 0, "rts", 0063, 0, "rtp", 0073, 0,
        /* input and output */
        "inp", 0101, 3, "out", 0121, 3};

#define NUMOPCODES (sizeof(opcodes) / sizeof(opcodes[0]))

/* The above rules in the table are:
 *  0:no argument needed,
 *  1:immediate byte follows
 *  2:two byte address follows
 *  3:output port number follows
 */

/*
 *
 *  This will take the input line, clean it up, remove
 *  comments, then break it into label (if exists) opcode, and
 *  arguments.
 *
 */

void parseline(char* line, char* label, char* opcode, char* arg1, char* arg2, int* args)
{
    int i;
    char cleanline[100], extrastuff[100], c;
    /* first clean line up a bit */
    strcpy(cleanline, line);
    for (i = 0; i < strlen(line); i++)
    {
        c = line[i];
        if ((c == ';') || (c == '/') || (c == 0x0A) || (c == '\n'))
        {
            cleanline[i] = 0;
            break;
        }
        /* if comma separates arguments, change to whitespace */

        if (c == ',')
            cleanline[i] = ' ';
        else
            cleanline[i] = c;
    }
    label[0] = opcode[0] = arg1[0] = arg2[0] = extrastuff[0] = 0;
    c = cleanline[0];
    if ((c != ' ') && (c != '\t') && (c != 0x00))
    {
        /* this is a label in column 0 */
        *args = sscanf(cleanline, "%s %s %s %s %s", label, opcode, arg1, arg2, extrastuff) - 2;
        if (*args == -1)
            opcode[0] = 0;

        if ((label[strlen(label) - 1] == ':') || (label[strlen(label) - 1] == ','))
            label[strlen(label) - 1] = 0; /* remove colon */
        else if (strcasecmp(label, "equ") != 0)
        {
            /* if label doesn't have colon, should be 'equ' opcode, else Warn */
            fprintf(stderr,
                    "WARNING: in line %d %s label %s lacking colon,and not 'equ' pseudoop.\n",
                    linecount, line, label);
        }
    }
    else
    {
        /*  "no label in column 0" */
        label[0] = 0;
        *args = sscanf(cleanline, "%s %s %s %s", opcode, arg1, arg2, extrastuff) - 1;
        if (*args < 0)
        {
            /* must just be comment line */
            opcode[0] = 0;
            return;
        }
    }
    if ((*args > 2) && (strcasecmp(opcode, "data") != 0))
    {
        printf("WARNING: extra text on line %d %s\n", linecount, line);
    }

    if (global_options.debug)
        printf("label=<%s> opcode=<%s> args=%d arg1=<%s> arg2=<%s> extra=<%s>\n", label, opcode,
               *args, arg1, arg2, extrastuff);
}

void definesymbol(char* symbol, int value)
{
    strcpy(symbols[numsymbols].label, symbol);
    symbols[numsymbols].value = value;
    numsymbols++;
}

int findsymbol(char* symbol)
{
    int i;
    for (i = 0; i < numsymbols; i++)
    {
        if (strcasecmp(symbols[i].label, symbol) == 0)
        {
            return (i);
        }
    }
    return (-1);
}

int findopcode(char* str)
{
    int i;
    for (i = 0; i < NUMOPCODES; i++)
        if (strcasecmp(str, opcodes[i].mnemonic) == 0)
            return (i);
    return (-1);
}

int evaluateargument(char* arg)
{
    int i, n, j, k;
    char part[4][20], operation[3], extra[80];
    int sum, val;
    int value[4];

    for (i = 0; i < 4; i++)
        part[i][0] = 0;
    for (i = 0; i < 3; i++)
        operation[i] = 0;
    extra[0] = 0;

    if (strncmp(arg, "\\HB\\", 4) == 0)
    {
        i = evaluateargument(arg + 4);
        return ((i >> 8) & 0xFF);
    }
    if (strncmp(arg, "\\LB\\", 4) == 0)
    {
        i = evaluateargument(arg + 4);
        return (i & 0xFF);
    }

    if (global_options.debug)
        printf("evaluating %s\n", arg);
    n = sscanf(arg, "%[^+-/*#]%[+-/*#]%[^+-/*#]%[+-/*#]%[^+-/*#]%[+-/*#]%[^+-/*#]%s", part[0],
               &operation[0], part[1], &operation[1], part[2], &operation[2], part[3], extra);
    if (global_options.debug)
        printf("n=%d part0=%s operation0=%c part1=%s operation1=%c part2=%s operation2=%c part3=%s "
               "extra=%s\n",
               n, part[0], operation[0], part[1], operation[1], part[2], operation[2], part[3],
               extra);

    if ((n % 2) == 0)
    {
        fprintf(stderr, "line %d can't evaluate last arg in %s\n", linecount, arg);
        exit(-1);
    }
    n = (n + 1) / 2;
    sum = 0;
    for (j = 0; j < n; j++)
    {

        if (isalpha(part[j][0]))
        {
            if ((i = findsymbol(part[j])) != -1)
            {
                val = symbols[i].value;
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
                fprintf(stderr, "in line %d unknown operation '%c'\n", linecount, operation[j - 1]);
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

void writebyte(int data, int address)
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

int finddata(char* line, int* outdata)
{
    char *ptr, c;
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
                    fprintf(stderr, " in line %d %s unknown escape sequence \\%c\n", linecount,
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
            fprintf(stderr, " in line %d %s max length is 12 bytes.\n", linecount, line);
            fprintf(stderr, " Use a second line for more data\n");
            exit(-1);
        }
        for (i = 0; i < n; i++)
        {
            *(outdata++) = evaluateargument(arg[i]);
        }
    }
    return (n);
}

int main(int argc, const char** argv)
{

    char outfilename[80], listfilename[80];
    char line[100], cleanline[100], label[20];
    char opcode[80], arg1str[20], arg2str[20], c, *cptr;
    char singlespacepad[9]; /* this is some extra padding if we use single space list file */
    int arg1, arg2, val, datalist[80], *ptr;
    int i, j, n, linecount, args, curaddress, lineaddress, code;
    int highbyte, lowbyte, maxport;
    /* all this just for a time */
    struct tm* timetm;
    time_t timet;
    time(&timet);
    timetm = localtime(&timet);
    cptr = asctime(timetm);

    try
    {
        global_options.parse(argc, argv);
    }
    catch (InvalidCommandLine&)
    {
        exit(-1);
    }

    if (global_options.single_byte_list)
        singlespacepad[0] = 0;
    else
        strcpy(singlespacepad, "        ");

    /* write either hex file or binary file */
    if (global_options.generate_binary_file)
        sprintf(outfilename, "%s.bin", global_options.input_filename_base.c_str());
    else
        sprintf(outfilename, "%s.hex", global_options.input_filename_base.c_str());
    sprintf(listfilename, "%s.lst", global_options.input_filename_base.c_str());
    if (global_options.debug)
        printf("global_options.input_filename_base.c_str()=\"%s\" infile=\"%s\" outfile=\"%s\" "
               "listfile=\"%s\"\n",
               global_options.input_filename_base.c_str(), global_options.input_filename.c_str(),
               outfilename, listfilename);
    /*
   *
   * Okay, now we have options processed, and we know file names.
   * Now let's open all files.
   *
   */

    if ((ifp = fopen(global_options.input_filename.c_str(), "rt")) == nullptr)
    {
        fprintf(stderr, "Can't open %s as input file\n", global_options.input_filename.c_str());
        exit(-1);
    }
    if (global_options.generate_binary_file)
    {
        if ((ofp = fopen(outfilename, "wb")) == nullptr)
        {
            fprintf(stderr, "Can't open %s as output file\n", outfilename);
            exit(-1);
        }
    }
    else
    {
        if ((ofp = fopen(outfilename, "wt")) == nullptr)
        {
            fprintf(stderr, "Can't open %s as output file\n", outfilename);
            exit(-1);
        }
    }
    if (global_options.generate_list_file)
    {
        if ((lfp = fopen(listfilename, "wt")) == nullptr)
        {
            fprintf(stderr, "Can't open %s as input file\n", listfilename);
            exit(-1);
        }
    }
    if (global_options.debug)
        printf("opened all files\n");

    /*
   *
   * Now initialize the symbol table, to get ready for assembly.
   *
   *
   */

    numsymbols = 0;

    /*
   *
   *  First pass, just parse through line, keep track of
   *  address, and build a symbol table
   *
   */

    if (global_options.debug || global_options.verbose)
        printf("Pass number One:  Read and Define Symbols\n");
    linecount = 0;
    curaddress = 0;
    fprintf(lfp, "AS8 assembler for intel 8008, t.e.jones Version 1.0\n");
    fprintf(lfp, "Options: listfile=%d debug=%d ", global_options.generate_list_file,
            global_options.debug);
    fprintf(lfp, "binaryout=%d singlelist=%d\n", global_options.generate_binary_file,
            global_options.single_byte_list);
    fprintf(lfp, "octalnums=%d markascii=%d\n", global_options.input_num_as_octal,
            global_options.mark_8_ascii);
    fprintf(lfp, "Infile=%s\n", global_options.input_filename.c_str());
    fprintf(lfp, "Assembly Performed: %s\n\n", cptr);
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

    while (fgets(line, 100, ifp))
    {
        lineaddress = curaddress;
        /* remove \n at end of line */
        if (strlen(line) > 0)
            line[strlen(line) - 1] = 0;
        linecount++;
        if (global_options.verbose || global_options.debug)
            printf("     0x%X \"%s\"\n", curaddress, line);
        /* this function breaks line into separate parts */
        parseline(line, label, opcode, arg1str, arg2str, &args);
        if (global_options.debug)
            printf("parsed line label=%s opcode=%s arg1str=%s\n", label, opcode, arg1str);
        if (label[0] != 0)
        {
            /* check to make sure not already defined */
            if ((i = findsymbol(label)) != -1)
            {
                fprintf(stderr, " in line %d %s label %s already defined as %d\n", linecount, line,
                        label, symbols[i].value);
                exit(-1);
            }
            /* define it */
            if ((strcasecmp(opcode, "equ") == 0) || (strcasecmp(opcode, "org") == 0))
                val = evaluateargument(arg1str);
            else
                val = curaddress;
            if (global_options.debug)
                printf("at address=%d=%X defining %s = %d =0x%X\n", curaddress, curaddress, label,
                       val, val);
            definesymbol(label, val);
        }

        if (opcode[0] == 0)
            continue;
        /* check if this opcode is one of the pseudoops */
        if (strcasecmp(opcode, "equ") == 0)
            continue;
        if (strcasecmp(opcode, "cpu") == 0)
        {
            if ((strcasecmp(arg1str, "8008") != 0) && (strcasecmp(arg1str, "i8008") != 0))
            {
                fprintf(stderr, " in line %d %s cpu only allowed is \"8008\" or \"i8008\"\n",
                        linecount, line);
                exit(-1);
            }
            continue;
        }

        if (strcasecmp(opcode, "org") == 0)
        {
            if ((curaddress = evaluateargument(arg1str)) == -1)
            {
                fprintf(stderr, " in line %d %s can't evaluate argument %s\n", linecount, line,
                        arg1str);
                exit(-1);
            }
        }
        else if (strcasecmp(opcode, "end") == 0)
        {
            continue;
        }
        else if (strcasecmp(opcode, "data") == 0)
        {
            n = finddata(line, datalist);
            if (global_options.debug)
                printf("got %d items in datalist\n", n);
            /* a negative number denotes that much space to save, but not specifying data */
            /* if so, just change sign to positive */
            if (n < 0)
                n = 0 - n;
            curaddress += n;
            continue;
        }
        /*
     *
     * Now we should have an opcode.
     *
     */
        else if ((i = findopcode(opcode)) != -1)
        {
            /* found the opcode */
            if (opcodes[i].rule == 0)
                curaddress += 1;
            else if (opcodes[i].rule == 1)
                curaddress += 2;
            else if (opcodes[i].rule == 2)
                curaddress += 3;
            else if (opcodes[i].rule == 3)
                curaddress += 1;
        }
        else
        {
            fprintf(stderr, " in line %d %s undefined opcode %s\n", linecount, line, opcode);
            exit(-1);
        }
    }

    /*
   *
   *  Okay, now hopefully have all symbols defined.  Do second pass.
   *
   */

    if (global_options.verbose || global_options.debug)
        printf("Pass number Two:  Re-read and assemble codes\n");
    rewind(ifp);
    linecount = 0;
    curaddress = 0;

    while (fgets(line, 100, ifp))
    {
        lineaddress = curaddress;
        if (strlen(line) > 0)
            line[strlen(line) - 1] = 0;
        linecount++;
        if (global_options.verbose || global_options.debug)
            printf("     0x%X \"%s\"\n", curaddress, line);
        /* this function breaks line into separate parts */
        parseline(line, label, opcode, arg1str, arg2str, &args);

        if (opcode[0] == 0)
        {
            /* must just be a comment line (or label only) */
            if (global_options.generate_list_file)
                fprintf(lfp, "%4d            %s%s\n", linecount, singlespacepad, line);
            continue;
        }
        /* check if this opcode is one of the pseudoops */
        if (strcasecmp(opcode, "equ") == 0)
        {
            if (global_options.generate_list_file)
                fprintf(lfp, "%4d            %s%s\n", linecount, singlespacepad, line);
            continue;
        }
        if (strcasecmp(opcode, "cpu") == 0)
        {
            if (global_options.generate_list_file)
                fprintf(lfp, "%4d            %s%s\n", linecount, singlespacepad, line);
            continue;
        }
        if (strcasecmp(opcode, "equ") == 0)
        {
            if (global_options.generate_list_file)
                fprintf(lfp, "%4d            %s%s\n", linecount, singlespacepad, line);
            continue;
        }
        if (strcasecmp(opcode, "org") == 0)
        {
            if ((curaddress = evaluateargument(arg1str)) == -1)
            {
                fprintf(stderr, " in line %d %s can't evaluate argument %s\n", linecount, line,
                        arg1str);
                exit(-1);
            }
            if (global_options.generate_list_file)
                fprintf(lfp, "%4d            %s%s\n", linecount, singlespacepad, line);
            continue;
        }
        if (strcasecmp(opcode, "end") == 0)
        {
            if (global_options.generate_list_file)
                fprintf(lfp, "%4d            %s%s\n", linecount, singlespacepad, line);
            /* could break here, but rather than break, */
            /* we will go ahead and check for more with a continue */
            continue;
        }
        else if (strcasecmp(opcode, "data") == 0)
        {
            n = finddata(line, datalist);
            /* if n is negative, that number of bytes are just reserved */
            if (n < 0)
            {
                if (global_options.generate_list_file)
                    fprintf(lfp, "%4d %02o-%03o     %s%s\n", linecount, ((lineaddress >> 8) & 0xFF),
                            (lineaddress & 0xFF), singlespacepad, line);
                curaddress += 0 - n;
                continue;
            }
            for (i = 0; i < n; i++)
                writebyte(datalist[i], curaddress++);
            if (global_options.generate_list_file)
            {
                if (global_options.single_byte_list)
                {
                    fprintf(lfp, "%4d %02o-%03o %03o %s\n", linecount, ((lineaddress >> 8) & 0xFF),
                            (lineaddress & 0xFF), datalist[0], line);
                    for (i = 1; i < n; i++)
                    {
                        fprintf(lfp, "%     %02o-%03o %03o\n", linecount,
                                (((lineaddress + i) >> 8) & 0xFF), ((lineaddress + i) & 0xFF),
                                datalist[i]);
                    }
                }
                else
                {
                    fprintf(lfp, "%4d %02o-%03o ", linecount, ((lineaddress >> 8) & 0xFF),
                            (lineaddress & 0xFF));
                    if (n == 1)
                        fprintf(lfp, "%03o          %s\n", datalist[0], line);
                    else if (n == 2)
                        fprintf(lfp, "%03o %03o      %s\n", datalist[0], datalist[1], line);
                    else if (n > 2)
                    {
                        fprintf(lfp, "%03o %03o %03o  %s\n", datalist[0], datalist[1], datalist[2],
                                line);
                        ptr = datalist + 3;
                        n -= 3;
                        lineaddress += 3;
                        while (n > 0)
                        {
                            /*	    fprintf(lfp,"            "); */
                            fprintf(lfp, "     %02o-%03o ", ((lineaddress >> 8) & 0xFF),
                                    (lineaddress & 0xFF));
                            if (n > 2)
                            {
                                fprintf(lfp, "%03o %03o %03o\n", ptr[0], ptr[1], ptr[2]);
                                ptr += 3;
                                n -= 3;
                                lineaddress += 3;
                            }
                            else
                            {
                                for (i = 0; i < n; i++)
                                {
                                    fprintf(lfp, "%03o ", ptr[0]);
                                    ptr++;
                                }
                                n = 0;
                                fprintf(lfp, "\n");
                            }
                        }
                    }
                }
            }
            continue;
        }
        /*
     *
     * Now we should have an opcode.
     *
     */
        else if ((i = findopcode(opcode)) == -1)
        {
            fprintf(stderr, " in line %d %s undefined opcode %s\n", linecount, line, opcode);
            exit(-1);
        }
        /* found the opcode */

        /* check that we have right number of arguments */
        if (((opcodes[i].rule == 0) && (args != 0)) || ((opcodes[i].rule == 1) && (args != 1)) ||
            ((opcodes[i].rule == 2) && (args != 1)) || ((opcodes[i].rule == 3) && (args != 1)))
        {
            fprintf(stderr, " in line %d %s we see an unexpected %d arguments\n", linecount, line,
                    args);
            exit(-1);
        }
        if (args == 1)
        {
            if ((arg1 = evaluateargument(arg1str)) == -1)
            {
                fprintf(stderr, " in line %d %s can't evaluate argument %s\n", linecount, line,
                        arg1str);
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

        if (opcodes[i].rule == 0)
        {
            /* single byte, no arguments */
            writebyte(opcodes[i].code, curaddress++);
            if (global_options.generate_list_file)
                fprintf(lfp, "%4d %02o-%03o %03o %s%s\n", linecount, ((lineaddress >> 8) & 0xFF),
                        (lineaddress & 0xFF), opcodes[i].code, singlespacepad, line);
        }
        else if (opcodes[i].rule == 1)
        {
            /* single byte, must follow */
            if ((arg1 > 255) || (arg1 < 0))
            {
                fprintf(stderr, " in line %d %s expected argument 0-255\n", linecount, line);
                fprintf(stderr, "    instead got %s=%d\n", arg1str, arg1);
                exit(-1);
            }
            code = opcodes[i].code;
            writebyte(code, curaddress++);
            writebyte(arg1, curaddress++);
            if (global_options.generate_list_file)
            {
                if (global_options.single_byte_list)
                {
                    fprintf(lfp, "%4d %02o-%03o %03o %s\n", linecount, ((lineaddress >> 8) & 0xFF),
                            (lineaddress & 0xFF), code, line);
                    lineaddress++;
                    fprintf(lfp, "     %02o-%03o %03o\n", (((lineaddress) >> 8) & 0xFF),
                            ((lineaddress) &0xFF), arg1);
                }
                else
                {
                    fprintf(lfp, "%4d %02o-%03o %03o %03o     %s\n", linecount,
                            ((lineaddress >> 8) & 0xFF), (lineaddress & 0xFF), code, arg1, line);
                }
            }
        }
        else if (opcodes[i].rule == 2)
        {
            /* two byte address to follow */
            if ((arg1 > 1024 * 16) || (arg1 < 0))
            {
                fprintf(stderr, " in line %d %s expected argument 0-%d\n", linecount, line,
                        1024 * 16);
                fprintf(stderr, "    instead got %s=%d\n", arg1str, arg1);
                exit(-1);
            }
            code = opcodes[i].code;
            lowbyte = (0xFF & arg1);
            highbyte = (0xFF & (arg1 >> 8));
            writebyte(code, curaddress++);
            writebyte(lowbyte, curaddress++);
            writebyte(highbyte, curaddress++);
            if (global_options.generate_list_file)
            {
                if (global_options.single_byte_list)
                {
                    fprintf(lfp, "%4d %02o-%03o %03o %s\n", linecount, ((lineaddress >> 8) & 0xFF),
                            (lineaddress & 0xFF), code, line);
                    lineaddress++;
                    fprintf(lfp, "     %02o-%03o %03o\n", ((lineaddress >> 8) & 0xFF),
                            (lineaddress & 0xFF), lowbyte);
                    lineaddress++;
                    fprintf(lfp, "     %02o-%03o %03o\n", ((lineaddress >> 8) & 0xFF),
                            (lineaddress & 0xFF), highbyte);
                }
                else
                {
                    fprintf(lfp, "%4d %02o-%03o %03o %03o %03o %s\n", linecount,
                            ((lineaddress >> 8) & 0xFF), (lineaddress & 0xFF), code, lowbyte,
                            highbyte, line);
                }
            }
        }
        else if (opcodes[i].rule == 3)
        {
            /* have an input or output instruction */
            if (opcodes[i].mnemonic[0] == 'i')
                maxport = 7;
            else
                maxport = 23;
            if ((arg1 > maxport) || (arg1 < 0))
            {
                fprintf(stderr, " in line %d %s expected port 0-%d\n", linecount, line, maxport);
                fprintf(stderr, "    instead got %s=%d\n", arg1str, arg1);
                exit(-1);
            }
            code = opcodes[i].code + (arg1 << 1);
            writebyte(code, curaddress++);
            if (global_options.generate_list_file)
                fprintf(lfp, "%4d %02o-%03o %03o %s%s\n", linecount, ((lineaddress >> 8) & 0xFF),
                        (lineaddress & 0xFF), code, singlespacepad, line);
        }
        else
        {
            fprintf(stderr, " in line %d %s can't comprehend rule %d\n", linecount, line,
                    opcodes[i].rule);
            exit(-1);
        }
    }
    /* signal to close off output file */
    writebyte(-1, -1);
    /* write symbol table to global_options.listfile */
    if (global_options.generate_list_file)
    {
        fprintf(lfp, "Symbol Count: %d\n", numsymbols);
        fprintf(lfp, "    Symbol  Oct Val  DecVal\n");
        fprintf(lfp, "    ------  -------  ------\n");
        for (i = 0; i < numsymbols; i++)
        {
            if (symbols[i].value > 255)
                fprintf(lfp, "%10s   %2o %03o  %5d\n", symbols[i].label,
                        ((symbols[i].value >> 8) & 0xFF), (symbols[i].value & 0xFF),
                        symbols[i].value);
            else
                fprintf(lfp, "%10s      %03o  %5d\n", symbols[i].label, symbols[i].value,
                        symbols[i].value);
        }
    }
}
