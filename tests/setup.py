from distutils.core import setup, Extension

strstr_wrapper_module = Extension(
    'strstr_wrappers',
    sources=['strstr_wrappers.c', '../fast_strstr.c', '../wordlen_strstr.c'],
    include_dirs=['../'],
)

setup(
    ext_modules = [strstr_wrapper_module],
)
