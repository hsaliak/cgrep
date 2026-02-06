import subprocess
import os
import unittest
import tempfile
import shutil

CGREP_BIN = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../build/cgrep"))

class TestCGrep(unittest.TestCase):
    def setUp(self):
        self.test_dir = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.test_dir)

    def run_cgrep(self, *args):
        result = subprocess.run([CGREP_BIN] + list(args), capture_output=True, text=True)
        return result

    def test_basic_search(self):
        path = os.path.join(self.test_dir, "hello.txt")
        with open(path, "w") as f:
            f.write("hello world\nthis is a test\ngrep me")
        
        res = self.run_cgrep("hello", path)
        self.assertEqual(res.returncode, 0)
        self.assertIn("hello world", res.stdout)
        self.assertNotIn("grep me", res.stdout)

    def test_recursive_search(self):
        subdir = os.path.join(self.test_dir, "subdir")
        os.mkdir(subdir)
        f1 = os.path.join(self.test_dir, "file1.txt")
        with open(f1, "w") as f:
            f.write("match this")
        f2 = os.path.join(subdir, "file2.txt")
        with open(f2, "w") as f:
            f.write("match this too")
        
        res = self.run_cgrep("-r", "match", self.test_dir)
        self.assertEqual(res.returncode, 0)
        self.assertIn("file1.txt:match this", res.stdout)
        self.assertIn("file2.txt:match this too", res.stdout)

    def test_line_numbers(self):
        path = os.path.join(self.test_dir, "lines.txt")
        with open(path, "w") as f:
            f.write("line 1\nline 2\nline 3")
        
        res = self.run_cgrep("-n", "line 2", path)
        self.assertEqual(res.returncode, 0)
        self.assertIn("2:line 2", res.stdout)

    def test_case_insensitive(self):
        path = os.path.join(self.test_dir, "case.txt")
        with open(path, "w") as f:
            f.write("HELLO WORLD")
        
        res = self.run_cgrep("-i", "hello", path)
        self.assertEqual(res.returncode, 0)
        self.assertIn("HELLO WORLD", res.stdout)

    def test_include_exclude(self):
        f1 = os.path.join(self.test_dir, "include.c")
        with open(f1, "w") as f: f.write("match")
        f2 = os.path.join(self.test_dir, "exclude.h")
        with open(f2, "w") as f: f.write("match")
        
        # Only include .c
        res = self.run_cgrep("-r", "--include", "*.c", "match", self.test_dir)
        self.assertIn("include.c", res.stdout)
        self.assertNotIn("exclude.h", res.stdout)
        
        # Exclude .h
        res = self.run_cgrep("-r", "--exclude", "*.h", "match", self.test_dir)
        self.assertIn("include.c", res.stdout)
        self.assertNotIn("exclude.h", res.stdout)

    def test_binary_skipping(self):
        path = os.path.join(self.test_dir, "binary.dat")
        with open(path, "wb") as f:
            f.write(b"text\0more text")
        
        res = self.run_cgrep("text", path)
        self.assertNotIn("binary.dat", res.stdout)

    def test_workers_flag(self):
        path = os.path.join(self.test_dir, "test.txt")
        with open(path, "w") as f:
            f.write("match")
        
        # Test with multiple workers
        res = self.run_cgrep("-w", "4", "match", path)
        self.assertEqual(res.returncode, 0)
        self.assertIn("match", res.stdout)

        # Test with invalid worker count
        res = self.run_cgrep("-w", "0", "match", path)
        self.assertNotEqual(res.returncode, 0)
        self.assertIn("Error: Number of workers must be at least 1", res.stderr)

if __name__ == "__main__":
    unittest.main()
