#ifndef INC_8008_ASSEMBLER_CONTEXT_H
#define INC_8008_ASSEMBLER_CONTEXT_H

#include "errors.h"
#include "options.h"
#include "symbol_table.h"

#include <memory>
#include <ostream>
#include <string_view>

class MacroContent;
class FileReader;

struct Context
{
    explicit Context(Options options);
    explicit Context(const std::shared_ptr<Context>& other_context);
    Context(Context&) = delete;
    ~Context();

    Options& get_options();
    [[nodiscard]] const Options& get_options() const;

    void define_symbol(std::string_view symbol_name, int value);
    [[nodiscard]] std::tuple<bool, int> get_symbol_value(std::string_view symbol_name) const;
    void list_symbols(std::ostream& output);

    enum ParsingMode
    {
        ACTIVE,
        CONDITIONAL_TRUE,
        CONDITIONAL_FALSE,
        MACRO_RECORDING,
    };

    [[nodiscard]] bool is_parsing_active() const;
    void set_parsing_mode(ParsingMode mode);
    [[nodiscard]] ParsingMode get_parsing_mode() const;

    /// Starts recording a macro in the context
    void start_macro(const std::string& macro_name, const std::vector<std::string>& arguments);

    /// Stops recoding the macro in the context and declares it in its parent context.
    void stop_macro();

    /// Records a line for the current macro
    void record_macro_line(const std::string& line);

    [[nodiscard]] bool has_macro(const std::string_view& macro_name) const;
    void* create_call_context(std::string_view macro_name,
                              const std::vector<std::string>& arguments,
                              FileReader& file_reader) const;
    void activate_macro(void* call_context);

private:
    const std::shared_ptr<Context> parent;

    Options options;
    SymbolTable symbol_table;
    ParsingMode parsing_mode{ACTIVE};
    std::unique_ptr<MacroContent> currently_recording_macro{};
    std::unordered_map<std::string, std::unique_ptr<MacroContent>> macros;

    void declare_macro(std::unique_ptr<MacroContent> macro_content);
};

class AlreadyDefinedMacro : public ExceptionWithReason
{
public:
    explicit AlreadyDefinedMacro(const std::string& macro_name);
};

#endif //INC_8008_ASSEMBLER_CONTEXT_H
