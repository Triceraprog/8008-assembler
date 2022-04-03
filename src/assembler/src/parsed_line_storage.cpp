#include "parsed_line_storage.h"

#include "context.h"
#include "files/file_reader.h"
#include "instruction.h"
#include "line_tokenizer.h"

void ParsedLineStorage::append_line(const Context& context, FileReader& file_reader,
                                    std::string_view input_line, std::size_t line_number,
                                    int address)
{
    LineTokenizer tokens = parse_line(context.options, input_line, line_number);
    {
        Instruction instruction{context, tokens.opcode, tokens.arguments, file_reader};
        parsed_lines.push_back(
                {line_number, address, tokens, std::move(instruction), std::string{input_line}});
    }
}

const ParsedLine& ParsedLineStorage::latest_line() const { return parsed_lines.back(); }

ParsedLineStorage::Iterator ParsedLineStorage::begin() { return std::begin(parsed_lines); }
ParsedLineStorage::Iterator ParsedLineStorage::end() { return std::end(parsed_lines); }
