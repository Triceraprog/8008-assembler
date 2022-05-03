#include "macro_content.h"

#include <sstream>
#include <ranges>

MacroContent::MacroContent(std::string_view name, const Parameters& parameters) : name{name}
{
    this->parameters.reserve(parameters.size());

    // Converts the parameters to lower case for future match.
    for(auto & param: parameters)
    {
        std::string lowercase(param.size(), ' ');
        std::transform(std::begin(param), std::end(param), std::begin(lowercase), ::tolower);

        this->parameters.push_back(lowercase);
    }
}

std::string_view MacroContent::get_name() const { return name; }

const MacroContent::Parameters& MacroContent::get_parameters() const { return parameters; }

void MacroContent::append_line(std::string_view line)
{
    content += line;
    content += '\n';
}

std::unique_ptr<std::istringstream> MacroContent::get_line_stream()
{
    return std::make_unique<std::istringstream>(content);
}
