#include "macro_content.h"

#include "files/file_reader.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(MacroContent, is_constructed_with_a_name_and_parameters)
{
    const std::vector<std::string> parameters;
    MacroContent macro{"my_macro", parameters};

    ASSERT_THAT(macro.get_name(), Eq("my_macro"));
    ASSERT_THAT(macro.get_parameters(), IsEmpty());
}

TEST(MacroContent, can_be_filled_with_lines)
{
    MacroContent macro{"my_macro", {}};

    macro.append_line("first line");
    macro.append_line("second line");

    auto stream = macro.get_line_stream();

    FileReader file_reader;
    file_reader.append(std::move(stream), std::string_view());

    std::vector<std::string> all_lines;
    std::copy(std::begin(file_reader), std::end(file_reader), std::back_inserter(all_lines));

    ASSERT_THAT(all_lines, SizeIs(2));
}
