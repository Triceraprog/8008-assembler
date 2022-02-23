#include "line_tokenizer.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(LineTokenizer, parse_empty_line)
{
    LineTokenizer tokenizer{""};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, IsEmpty());
    ASSERT_THAT(tokenizer.arguments, IsEmpty());
}

TEST(LineTokenizer, parse_label_only)
{
    LineTokenizer tokenizer{"LABEL:"};
    ASSERT_THAT(tokenizer.label, Eq("LABEL"));
    ASSERT_THAT(tokenizer.opcode, IsEmpty());
    ASSERT_THAT(tokenizer.arguments, IsEmpty());
}

TEST(LineTokenizer, parse_opcode_only)
{
    LineTokenizer tokenizer{" ORG"};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, Eq("ORG"));
    ASSERT_THAT(tokenizer.arguments, IsEmpty());
}

TEST(LineTokenizer, parse_label_with_opcode)
{
    LineTokenizer tokenizer{"START: LAA"};
    ASSERT_THAT(tokenizer.label, Eq("START"));
    ASSERT_THAT(tokenizer.opcode, Eq("LAA"));
    ASSERT_THAT(tokenizer.arguments, IsEmpty());
}

TEST(LineTokenizer, parse_opcode_with_one_argument)
{
    LineTokenizer tokenizer{" LAI $ff"};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, Eq("LAI"));
    ASSERT_THAT(tokenizer.arguments, SizeIs(1));
    ASSERT_THAT(tokenizer.arguments[0], Eq("$ff"));
}

TEST(LineTokenizer, parse_opcode_with_two_arguments)
{
    LineTokenizer tokenizer{" MOV A,$01"};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, Eq("MOV"));
    ASSERT_THAT(tokenizer.arguments, SizeIs(2));
    ASSERT_THAT(tokenizer.arguments[0], Eq("A"));
    ASSERT_THAT(tokenizer.arguments[1], Eq("$01"));
}

TEST(LineTokenizer, parse_opcode_with_two_arguments_and_label)
{
    LineTokenizer tokenizer{"MOV: MOV A,$01"};
    ASSERT_THAT(tokenizer.label, Eq("MOV"));
    ASSERT_THAT(tokenizer.opcode, Eq("MOV"));
    ASSERT_THAT(tokenizer.arguments, SizeIs(2));
    ASSERT_THAT(tokenizer.arguments[0], Eq("A"));
    ASSERT_THAT(tokenizer.arguments[1], Eq("$01"));
}

TEST(LineTokenizer, parse_data_integer_arguments)
{
    LineTokenizer tokenizer{" DATA 1,010,$30,40"};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, Eq("DATA"));
    ASSERT_THAT(tokenizer.arguments, SizeIs(4));
    ASSERT_THAT(tokenizer.arguments[0], Eq("1"));
    ASSERT_THAT(tokenizer.arguments[1], Eq("010"));
    ASSERT_THAT(tokenizer.arguments[2], Eq("$30"));
    ASSERT_THAT(tokenizer.arguments[3], Eq("40"));
}

TEST(LineTokenizer, parse_data_reservation)
{
    LineTokenizer tokenizer{" DATA *4"};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, Eq("DATA"));
    ASSERT_THAT(tokenizer.arguments, SizeIs(1));
    ASSERT_THAT(tokenizer.arguments[0], Eq("*4"));
}

TEST(LineTokenizer, parse_string_with_spaces)
{
    LineTokenizer tokenizer{" DATA \" STRING \""};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, Eq("DATA"));
    ASSERT_THAT(tokenizer.arguments, SizeIs(1));
    ASSERT_THAT(tokenizer.arguments[0], Eq("\" STRING \""));
}

TEST(LineTokenizer, parse_two_strings)
{
    LineTokenizer tokenizer{R"( DATA "STRING1","STRING2")"};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, Eq("DATA"));
    ASSERT_THAT(tokenizer.arguments, SizeIs(2));
    ASSERT_THAT(tokenizer.arguments[0], Eq("\"STRING1\""));
    ASSERT_THAT(tokenizer.arguments[1], Eq("\"STRING2\""));
}

TEST(LineTokenizer, parses_char)
{
    LineTokenizer tokenizer{" DATA 'A'"};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, Eq("DATA"));
    ASSERT_THAT(tokenizer.arguments, SizeIs(1));
    ASSERT_THAT(tokenizer.arguments[0], Eq("'A'"));
}

TEST(LineTokenizer, parses_escaped_quote)
{
    LineTokenizer tokenizer{" DATA '\"'"};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, Eq("DATA"));
    ASSERT_THAT(tokenizer.arguments, SizeIs(1));
    ASSERT_THAT(tokenizer.arguments[0], Eq("'\"'"));
}

TEST(LineTokenizer, parses_string_with_comma)
{
    LineTokenizer tokenizer{R"( DATA "D1,D2")"};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, Eq("DATA"));
    ASSERT_THAT(tokenizer.arguments, SizeIs(1));
    ASSERT_THAT(tokenizer.arguments[0], Eq("\"D1,D2\""));
}

TEST(LineTokenizer, extracts_single_comment)
{
    LineTokenizer tokenizer{"; Comment from the start of the line"};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, IsEmpty());
    ASSERT_THAT(tokenizer.arguments, SizeIs(0));
    ASSERT_THAT(tokenizer.comment, Eq("; Comment from the start of the line"));
}

TEST(LineTokenizer, extracts_single_comment_after_column_zero)
{
    LineTokenizer tokenizer{" ; Comment after_column_0"};
    ASSERT_THAT(tokenizer.label, IsEmpty());
    ASSERT_THAT(tokenizer.opcode, IsEmpty());
    ASSERT_THAT(tokenizer.arguments, SizeIs(0));
    ASSERT_THAT(tokenizer.comment, Eq("; Comment after_column_0"));
}
