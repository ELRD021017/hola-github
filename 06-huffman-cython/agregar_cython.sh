#!/usr/bin/env bash
# Agrega la carpeta 06-huffman-cython al repositorio y actualiza README y .gitignore.
#
# Uso (dentro de la carpeta del repositorio, despues de git pull):
#   bash agregar_cython.sh <carpeta_con_los_archivos_descargados>
#
# La carpeta debe tener (todos sueltos, sin subcarpetas):
#   huffman_cy.pyx  setup.py  comprimir_cy.py  descomprimir_cy.py  benchmark.py
#   README_06.md  (pasa a ser 06-huffman-cython/README.md)
#   README.md     (reemplaza el README de la raiz)
set -e

ORIGEN="${1:?Indica la carpeta con los archivos descargados}"
DESTINO="06-huffman-cython"
mkdir -p "$DESTINO"

copiar() {   # copiar <archivo en ORIGEN> <ruta destino>
  if [ -f "$ORIGEN/$1" ]; then cp "$ORIGEN/$1" "$2"; echo "copiado: $1 -> $2"
  else echo "FALTA:   $ORIGEN/$1"; fi
}

for f in huffman_cy.pyx setup.py comprimir_cy.py descomprimir_cy.py benchmark.py; do
  copiar "$f" "$DESTINO/$f"
done
copiar README_06.md "$DESTINO/README.md"
copiar README.md README.md

# .gitignore: lo que genera la compilacion de Cython (sin ignorar los .c de las otras carpetas)
touch .gitignore
for linea in "$DESTINO/huffman_cy.c" "$DESTINO/build/" "*.so" "*.pyd" "__pycache__/" ".venv/" "*.huf"; do
  grep -qxF "$linea" .gitignore || echo "$linea" >> .gitignore
done

git add -A
echo
git status --short
echo
echo "Si todo esta bien:"
echo "  git commit -m \"Agregar Huffman en Cython y comparacion con C\""
echo "  git push"
