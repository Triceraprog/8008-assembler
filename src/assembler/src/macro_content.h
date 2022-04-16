#ifndef INC_8008_ASSEMBLER_MACRO_CONTENT_H
#define INC_8008_ASSEMBLER_MACRO_CONTENT_H

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class MacroContent
{
public:
    MacroContent(std::string_view name, const std::vector<std::string>& parameters);

    [[nodiscard]] std::string_view get_name() const;
    [[nodiscard]] const std::vector<std::string>& get_parameters() const;

    void append_line(std::string_view line);

    std::unique_ptr<std::istringstream> get_line_stream();

private:
    std::string name;
    std::vector<std::string> parameters;

    std::string content;
};

#endif //INC_8008_ASSEMBLER_MACRO_CONTENT_H
