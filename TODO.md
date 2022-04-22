- Macro system
  - Recording the macro: add to the current recording context (ParsedLine's? -> raw lines)
  - Needs a way to inject the macro content with the binding context when it's called
    - Add the MacroCallContext, which binds the MacroContent and the Arguments.
    - The call context as a MacroContent and a line number
    - Lines can be fetched and will be transformed. The latest line is kept into the
      - No need if injected through FileReader
    Call Context (shared_ptr to protect against the caller).
    - When a Content is exhausted, then the Call Context is poped.
    - When there's no more call content, it gives nulled ParsedLine
  - Be careful of the line number with Macro. Maybe some stacked line number scheme.
  - Note: remember, ParsedLines are associated to Contexts, so locality works.
  - Recording ParsedLines seems too much, as the Context won't be ok.
    - In a first step, record plain lines.
    - How to pop the context though ? Either injecting a push/pop context or
    have a callback at the end of the FileReader. The first can be simpler.
    A name tag associated to the macro can be used.
- Simplify file declaration in tests (based on a naming scheme)
- The responsibility of the short format should probably be in Listing
- The listing can be reconstructed without keeping the initial line.
- Option to output an expended, simple assembly code with a syntax for another assembler, especially as8
- Check name of executable and if as8, enters a compatible mode if needed
- If the accumulator results in a negative number, various weird things happen, because it's passed as int (wrong hex, wrong lst)
- Support for priorities in expressions (with optional compatibilities with as8)
- Change the hardcoded path in functional tests to a parameter and generate the call to tests from CMake
- If the push/pop contexts have performance problems with copying the options, copy on write could be used.
