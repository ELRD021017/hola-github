#!/usr/bin/env python3
"""Uso: python comprimir_cy.py entrada salida.huf   (requiere compilar huffman_cy antes)"""
import sys

try:
    import huffman_cy
except ImportError:
    sys.exit("No se encontro el modulo huffman_cy. Compilalo con:\n"
             "  python setup.py build_ext --inplace")

if len(sys.argv) != 3:
    sys.exit("Uso: python comprimir_cy.py entrada salida.huf")

try:
    original, comprimido = huffman_cy.comprimir(sys.argv[1], sys.argv[2])
except (OSError, ValueError) as e:
    sys.exit(f"Error: {e}")
print(f"{sys.argv[1]}: {original} bytes -> {sys.argv[2]}: {comprimido} bytes")
