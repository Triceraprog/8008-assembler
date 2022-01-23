#include "utils.h"

#include <cstring>
#include <cctype>
#include <algorithm>

bool ci_equals(const std::string_view& lhs, const std::string_view& rhs)
{
    return strcasecmp(lhs.data(), rhs.data()) == 0;
}

std::string_view left_trim_string(std::string_view str)
{
    auto it = std::find_if_not(str.begin(), str.end(), [](auto c) { return std::isspace(c); });
    if (it != str.end())
    {
        str.remove_prefix(std::distance(str.begin(), it));
    }

    return str;
}

std::string_view right_trim_string(std::string_view str)
{
    auto it = std::find_if_not(str.rbegin(), str.rend(), [](auto c) { return std::isspace(c); });
    if (it != str.rend())
    {
        str.remove_suffix(std::distance(str.rbegin(), it));
    }

    return str;
}

std::string_view trim_string(std::string_view str)
{
    return right_trim_string(left_trim_string(str));
}
