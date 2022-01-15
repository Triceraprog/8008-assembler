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


def file_equal_binary(ref_file, file_):
    with open(file_, "rb") as f:
        with open(ref_file, "rb") as ref_f:
            file_content = f.read()
            ref_file_content = ref_f.read()

            return len(file_content) == len(ref_file_content) and file_content == ref_file_content


@contextlib.contextmanager
def temp_files(list_of_paths):
    def unlink_files(files):
        for path in files:
            if path.exists() and path.is_file():
                path.unlink()

    unlink_files(list_of_paths)
    try:
        yield None
    finally:
        unlink_files(list_of_paths)


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


class DataFiles:
    def __init__(self):
        data_path = pathlib.Path("data")
        self.input_file = data_path.joinpath("basics.asm")
        self.output_hex_file = data_path.joinpath("basics.hex")
        self.output_lst_file = data_path.joinpath("basics.lst")
        self.output_bin_file = data_path.joinpath("basics.bin")
        self.output_hex_ref_file = data_path.joinpath("ref_basics.hex")
        self.output_hex_octal_ref_file = data_path.joinpath("ref_basics_octal.hex")
        self.output_bin_ref_file = data_path.joinpath("ref_basics.bin")
        self.output_lst_ref_file = data_path.joinpath("ref_basics.lst")
        self.output_lst_debug_ref_file = data_path.joinpath("ref_basics_debug.lst")
        self.output_lst_bin_ref_file = data_path.joinpath("ref_basics_bin.lst")
        self.output_lst_octal_ref_file = data_path.joinpath("ref_basics_octal.lst")
        self.output_lst_single_ref_file = data_path.joinpath("ref_basics_single.lst")

        self.temp_files = [self.output_lst_file, self.output_hex_file, self.output_bin_file]


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
        files = DataFiles()

        with temp_files(files.temp_files):
            result = run_assembler([files.input_file])

            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stdout, '')
            self.assertEqual(result.stderr, '')

            self.assertTrue(files.output_hex_file.is_file())
            self.assertTrue(files.output_lst_file.is_file())
            self.assertFalse(files.output_bin_file.is_file())

            self.assertTrue(file_equal(files.output_hex_ref_file, files.output_hex_file),
                            msg=f"File differs {files.output_hex_file}")
            self.assertTrue(file_equal(files.output_lst_ref_file, files.output_lst_file),
                            msg=f"File differs {files.output_lst_file}")

    @unittest.skip  # This option makes the assembler crash
    def test_assemble_a_file_with_no_output_listing(self):
        files = DataFiles()

        with temp_files(files.temp_files):
            result = run_assembler(["-nl", files.input_file])

            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stdout, '')
            self.assertEqual(result.stderr, '')

            self.assertTrue(files.output_hex_file.is_file())
            self.assertFalse(files.output_lst_file.is_file())
            self.assertFalse(files.output_bin_file.is_file())

    def test_assemble_a_file_with_output_debug(self):
        files = DataFiles()

        with temp_files(files.temp_files):
            result = run_assembler(["-d", files.input_file])

            self.assertEqual(result.returncode, 0)
            self.assertNotEqual(result.stdout, '')  # Don't care much about what's the debug output
            self.assertEqual(result.stderr, '')

            self.assertTrue(files.output_hex_file.is_file())
            self.assertTrue(files.output_lst_file.is_file())
            self.assertFalse(files.output_bin_file.is_file())

            self.assertTrue(file_equal(files.output_hex_ref_file, files.output_hex_file),
                            msg=f"File differs {files.output_hex_file}")
            self.assertTrue(file_equal(files.output_lst_debug_ref_file, files.output_lst_file),
                            msg=f"File differs {files.output_lst_file}")

    def test_assemble_a_file_with_output_binary(self):
        files = DataFiles()

        with temp_files(files.temp_files):
            result = run_assembler(["-bin", files.input_file])

            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stdout, '')
            self.assertEqual(result.stderr, '')

            self.assertFalse(files.output_hex_file.is_file())
            self.assertTrue(files.output_lst_file.is_file())
            self.assertTrue(files.output_bin_file.is_file())

            self.assertTrue(file_equal_binary(files.output_bin_ref_file, files.output_bin_file),
                            msg=f"File differs {files.output_bin_file}")
            self.assertTrue(file_equal(files.output_lst_bin_ref_file, files.output_lst_file),
                            msg=f"File differs {files.output_lst_file}")

    def test_assemble_a_file_with_octal_as_default(self):
        files = DataFiles()

        with temp_files(files.temp_files):
            result = run_assembler(["-octal", files.input_file])

            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stdout, '')
            self.assertEqual(result.stderr, '')

            self.assertTrue(files.output_hex_file.is_file())
            self.assertTrue(files.output_lst_file.is_file())
            self.assertFalse(files.output_bin_file.is_file())

            self.assertTrue(file_equal(files.output_hex_octal_ref_file, files.output_hex_file),
                            msg=f"File differs {files.output_hex_file}")
            self.assertTrue(file_equal(files.output_lst_octal_ref_file, files.output_lst_file),
                            msg=f"File differs {files.output_lst_file}")

    def test_assemble_a_file_with_single_byte_output(self):
        files = DataFiles()

        with temp_files(files.temp_files):
            result = run_assembler(["-single", files.input_file])

            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stdout, '')
            self.assertEqual(result.stderr, '')

            self.assertTrue(files.output_hex_file.is_file())
            self.assertTrue(files.output_lst_file.is_file())
            self.assertFalse(files.output_bin_file.is_file())

            self.assertTrue(file_equal(files.output_hex_ref_file, files.output_hex_file),
                            msg=f"File differs {files.output_hex_file}")
            self.assertTrue(file_equal(files.output_lst_single_ref_file, files.output_lst_file),
                            msg=f"File differs {files.output_lst_file}")

# Next tets
# DONE - Assembly of a file, check the output
# DONE - Assembly of a file, check the output with -nl
# DONE - Assembly of a file, check the output with -d
# DONE - Assembly of a file, check the output with -bin
# DONE - Assembly of a file, check the output with -octal
# DONE - Assembly of a file, check the output with -single
# - Assembly of a file, check the output with -markascii
# Then a big test with the whole SCELBAL, for compatibility (probably check if the file is there not to distribute it)
# When the assembler will be done and ROM rewritten, then make the test on them too
