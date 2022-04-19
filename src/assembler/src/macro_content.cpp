#include "macro_content.h"

#include <sstream>

MacroContent::MacroContent(std::string_view name, const Parameters& parameters) : name{name}
{
    this->parameters.reserve(parameters.size());
    std::copy(parameters.begin(), parameters.end(), std::back_inserter(this->parameters));
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
