import unittest
from strstr_wrappers import py_fast_strstr, py_wordlen_strstr


class TestStrStrBase(object):
    def search(self, haystack, needle):
        raise NotImplementedError

    def do_test(self, haystack, needle):
        if isinstance(haystack, str):
            haystack = haystack.encode('ascii')
        if isinstance(needle, str):
            needle = needle.encode('ascii')
        return self.assertEqual(
            self.search(haystack, needle),
            haystack.find(needle),
        )

    def test_both_empty(self):
        self.do_test('', '')

    def test_empty_haystack(self):
        self.do_test('', 'x')

    def test_empty_needle(self):
        self.do_test('x', '')

    def test_identical(self):
        self.assertEqual(self.search(b'abc', b'abc'), 0)

    def test_found_at_middle(self):
        haystack = b'XXXneedleXXX'
        needle = b'needle'
        self.assertEqual(self.search(haystack, needle), 3)

    def test_found_at_start(self):
        haystack = b'needleXXX'
        needle = b'needle'
        self.assertEqual(self.search(haystack, needle), 0)

    def test_found_at_end(self):
        haystack = b'XXXneedle'
        needle = b'needle'
        self.assertEqual(self.search(haystack, needle), 3)

    def test_found_multiple_times(self):
        haystack = b'XXXneedleXXXneedleXXXneedleXXX'
        needle = b'needle'
        self.assertEqual(self.search(haystack, needle), 3)

    def test_not_past_null_byte(self):
        haystack = b'XXX\0needleXXX'
        needle = b'needle'
        self.assertEqual(self.search(haystack, needle), -1)

    def test_random_identical(self):
        import random
        for n_test in range(100):
            length = random.randint(0, 1000)
            haystack = ''.join([
                chr(random.randint(1, 127))
                for _i in range(length)
            ])
            needle = haystack
            self.do_test(haystack, needle)

    def test_random(self):
        import random
        for n_test in range(100):
            haystack = ''.join([
                chr(random.randint(1, 127))
                for _i in range(random.randint(0, 1000))
            ])
            needle = ''.join([
                chr(random.randint(1, 127))
                for _i in range(random.randint(0, 200))
            ])
            self.do_test(haystack, needle)


class TestFastStrStr(TestStrStrBase, unittest.TestCase):
    def search(self, haystack, needle):
        return py_fast_strstr(haystack, needle)


class TestWordlenStrStr(TestStrStrBase, unittest.TestCase):
    def search(self, haystack, needle):
        return py_wordlen_strstr(haystack, needle)
