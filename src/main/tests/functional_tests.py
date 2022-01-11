import unittest
import pathlib

# Path is hardcoded path at the moment
path = "../../../cmake-build-debug/src/main"
assembler_path = pathlib.Path(path).joinpath("as-8008")


def run_assembler():
    from subprocess import run, PIPE
    result = run([assembler_path],
                 stdin=None, stdout=PIPE, stderr=PIPE,
                 encoding="UTF-8", text=True,
                 timeout=10, check=False)

    return result


help_output_first_line_words = ["Usage: ", "[options] infile"]
help_output = """    where <infile> is assembly code file, extension defaults to .asm
    and options include...
    -v        verbose output
    -nl       no list (default is to make .lst file.)
    -d        debug assembler (extra output)
    -bin      makes output binary ROM file, otherwise intel hex
    -octal    makes unidentified 3-digit numbers octal (default decimal)
    -single   makes .lst file single byte per line, otherwise 3/line.
    -markascii makes highest bit in ascii bytes a one (mark).
"""


class TestFunctional(unittest.TestCase):
    def test_without_arguments_help_is_display(self):
        from os import linesep

        result = run_assembler()
        self.assertEqual(result.returncode, 255)
        self.assertEqual(result.stdout.strip(), "")

        stderr_lines = result.stderr.split(linesep)
        expected_lines = help_output.split(linesep)

        for word in help_output_first_line_words:
            self.assertIn(word, stderr_lines[0])
            self.assertIn(word, stderr_lines[0])

        for line, expected in zip(stderr_lines[1:], expected_lines):
            self.assertEqual(line, expected)
