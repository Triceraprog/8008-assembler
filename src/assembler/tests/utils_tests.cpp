#include "utils.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(TrimmingString, trims_tabs)
{
    std::string source{"\t\t100\t\t\t"};

    const auto& destination = trim_string(source);

    ASSERT_THAT(destination, Eq("100"));
}

TEST(TrimmingString, trims_nothing)
{
    std::string source{"100"};

    const auto& destination = trim_string(source);

    ASSERT_THAT(destination, Eq("100"));
}
