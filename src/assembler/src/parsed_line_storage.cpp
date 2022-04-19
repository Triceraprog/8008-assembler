#include "parsed_line_storage.h"

#include "context.h"
#include "files/file_reader.h"
#include "instruction.h"
#include "line_tokenizer.h"

void ParsedLineStorage::append_line(const std::shared_ptr<Context>& context,
                                    FileReader& file_reader, std::string_view input_line,
                                    std::size_t line_number, int address)
{
    auto name_tag_ref = get_name_tag_ref(file_reader.get_name_tag());

    LineTokenizer tokens = parse_line(context->get_options(), input_line, line_number);
    Instruction instruction{*context, tokens.label, tokens.opcode, tokens.arguments, file_reader};
    parsed_lines.push_back({line_number, address, tokens, std::move(instruction),
                            std::string{input_line}, name_tag_ref, context});
}

std::shared_ptr<std::string> ParsedLineStorage::get_name_tag_ref(const std::string& name_tag)
{
    auto where = std::find_if(std::begin(name_tags), std::end(name_tags),
                              [&name_tag](auto& name) { return *name == name_tag; });

    std::shared_ptr<std::string> name_tag_ref;
    if (where != std::end(name_tags))
    {
        name_tag_ref = *where;
    }
    else
    {
        name_tags.push_back(std::make_shared<std::string>(name_tag));
        name_tag_ref = name_tags.back();
    }
    return name_tag_ref;
}

const ParsedLine& ParsedLineStorage::latest_line() const { return parsed_lines.back(); }

ParsedLineStorage::Iterator ParsedLineStorage::begin() { return std::begin(parsed_lines); }
ParsedLineStorage::Iterator ParsedLineStorage::end() { return std::end(parsed_lines); }
