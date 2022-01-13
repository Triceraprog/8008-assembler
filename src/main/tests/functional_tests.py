import contextlib
import unittest
import pathlib

# Path is hardcoded path at the moment
exe_path = "../../../cmake-build-debug/src/main"
assembler_path = pathlib.Path(exe_path).joinpath("as-8008")


def run_assembler(arg_list=None):
    from subprocess import run, PIPE

    arg_list = arg_list if arg_list else []
    result = run([assembler_path] + arg_list,
                 stdin=None, stdout=PIPE, stderr=PIPE,
                 encoding="UTF-8", text=True,
                 timeout=10, check=False)

    return result


def file_equal(ref_file, file_):
    with open(file_, "rt") as f:
        with open(ref_file, "rt") as ref_f:
            file_content = f.readlines()
            ref_file_content = ref_f.readlines()

            difference_found = False

            for l1, l2 in zip(ref_file_content, file_content):
                if ASSEMBLY_TEXT in l1 and ASSEMBLY_TEXT in l2:
                    continue
                elif l1 != l2:
                    difference_found = True
                    print("Difference in files")
                    print(l1)
                    print("----")
                    print(l2)
                    print("----")

            return not difference_found


@contextlib.contextmanager
def temp_files(list_of_paths):
    for path in list_of_paths:
        if path.exists() and path.is_file():
            path.unlink()
    try:
        yield None
    finally:
        for path in list_of_paths:
            if path.exists() and path.is_file():
                path.unlink()


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

ASSEMBLY_TEXT = "Assembly Performed"


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

    def test_assemble_a_file(self):
        data_path = pathlib.Path("data")
        input_file = data_path.joinpath("basics.asm")
        output_hex_file = data_path.joinpath("basics.hex")
        output_lst_file = data_path.joinpath("basics.lst")

        with temp_files([output_hex_file, output_lst_file]):
            result = run_assembler([input_file])

            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stdout, '')
            self.assertEqual(result.stderr, '')

            self.assertTrue(output_hex_file.is_file())
            self.assertTrue(output_lst_file.is_file())

            output_hex_ref_file = data_path.joinpath("ref_basics.hex")
            output_lst_ref_file = data_path.joinpath("ref_basics.lst")

            self.assertTrue(file_equal(output_hex_ref_file, output_hex_file), msg=f"File differs {output_hex_file}")
            self.assertTrue(file_equal(output_lst_ref_file, output_lst_file), msg=f"File differs {output_lst_file}")

# Next tets
# DONE - Assembly of a file, check the output
# - Assembly of a file, check the output with -nl
# - Assembly of a file, check the output with -d
# - Assembly of a file, check the output with -bin
# - Assembly of a file, check the output with -octal
# - Assembly of a file, check the output with -single
# - Assembly of a file, check the output with -markascii
# The checks are both on the binary output and the .lst
# On a small file
# Then a big test with the whole SCELBAL, for compatibility (probably check if the file is there not to distribute it)
# When the assembler will be done and ROM rewritten, then make the test on them too
