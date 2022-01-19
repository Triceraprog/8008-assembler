#include "utils.h"

#include <cstring>

bool ci_equals(const std::string_view& lhs, const std::string_view& rhs)
{
    return strcasecmp(lhs.data(), rhs.data()) == 0;
}
