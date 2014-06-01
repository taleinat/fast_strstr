cdef extern from "../fast_strstr.h":
    char *fast_strstr(const char *haystack, const char *needle)

cdef extern from "../wordlen_strstr.h":
    char* wordlen_strstr(const char *haystack, const char *needle)


__all__ = ['py_fast_strstr', 'py_wordlen_strstr']


def py_fast_strstr(haystack, needle):
    cdef char *c_haystack = haystack
    cdef char *c_needle = needle
    cdef char *result
    result = fast_strstr(c_haystack, c_needle)
    return (result - c_haystack) if result != NULL else -1


def py_wordlen_strstr(haystack, needle):
    cdef char *c_haystack = haystack
    cdef char *c_needle = needle
    cdef char *result
    result = wordlen_strstr(c_haystack, c_needle)
    return (result - c_haystack) if result != NULL else -1
