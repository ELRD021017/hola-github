# Compilar con:  python setup.py build_ext --inplace
from setuptools import setup, Extension
from Cython.Build import cythonize

extensiones = [
    Extension("huffman_cy", ["huffman_cy.pyx"], extra_compile_args=["-O3"]),
]

setup(
    name="huffman_cy",
    ext_modules=cythonize(extensiones, compiler_directives={"language_level": "3"}),
)
