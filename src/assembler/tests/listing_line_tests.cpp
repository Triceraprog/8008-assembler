#include "listing_line.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(ListingLine, with_line_and_text)
{
    auto line = ListingLine(1);
    line.add_line_content("; Comment");

    ASSERT_THAT(line.str(), Eq("   1                    ; Comment"));
}

TEST(ListingLine, with_line_address_and_opcode)
{
    auto line = ListingLine(9999, 1);
    line.add_byte(0300);
    line.add_line_content("LAA");

    ASSERT_THAT(line.str(), Eq("9999 00-001 300         LAA"));
}

TEST(ListingLine, with_line_address_and_three_bytes)
{
    auto line = ListingLine(1234, 4179);
    line.add_byte(0100);
    line.add_byte(0200);
    line.add_byte(0300);
    line.add_line_content("DATA 100,200,300");

    ASSERT_THAT(line.str(), Eq("1234 20-123 100 200 300 DATA 100,200,300"));
}
