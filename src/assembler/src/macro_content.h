#ifndef INC_8008_ASSEMBLER_MACRO_CONTENT_H
#define INC_8008_ASSEMBLER_MACRO_CONTENT_H

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class MacroContent
{
public:
    using Parameters = std::vector<std::string>;
    MacroContent(std::string_view name, const Parameters& parameters);

    [[nodiscard]] std::string_view get_name() const;
    [[nodiscard]] const Parameters& get_parameters() const;

    void append_line(std::string_view line);

    std::unique_ptr<std::istringstream> get_line_stream();

private:
    std::string name;
    Parameters parameters;

    std::string content;
};

#endif //INC_8008_ASSEMBLER_MACRO_CONTENT_H
