# Hi GitHub

Repositorio de la clase de computación. Aquí guardo los programas que voy haciendo, organizados por carpetas.

## Estructura del repositorio

| Carpeta | Contenido |
|---|---|
| `01-introduccion/` | `hola.c`: primer programa en C que imprime un mensaje en pantalla. |
| `02-estructuras-datos/` | `fifo.c` (cola FIFO con listas doblemente ligadas), `lifo.c` (pila LIFO con listas doblemente ligadas) y `hash.c` (tabla hash). |
| `03-algoritmos-numericos/` | `horner_float.c` (evaluación de polinomios con el método de Horner) y `mulx128b.c`. |
| `04-algoritmo-genetico/` | `ag_rastrigin_32bits.c`: algoritmo genético con codificación binaria que minimiza la función de Rastrigin de dos variables. |
| `05-huffman/` | Compresión y descompresión con el algoritmo de Huffman en C: `huffman.c`, `huffman_comprimir.c` y `huffman_descomprimir.c`. |
| `06-huffman-cython/` | El mismo Huffman escrito en Cython (con el mismo formato de archivo que la versión en C) y un benchmark que compara el tiempo de C contra Cython. |

## Última actualización

Se añadió la carpeta **`06-huffman-cython/`**:

- `huffman_cy.pyx`: compresor y descompresor de Huffman en Cython, compatible con los archivos que genera la versión en C.
- `benchmark.py`: compila los programas en C, mide el tiempo de C y de Cython con distintos tamaños de archivo y comprueba que ambos produzcan archivos idénticos.

### Actualizaciones anteriores

- Algoritmo genético en `04-algoritmo-genetico/`: individuos de 64 bits (32 bits para `x` y 32 para `y`), población de 50, cruza 0.9 y mutación 0.1, con `qsort` para quedarse con los mejores.
- Huffman en C en `05-huffman/`: `huffman.c` (ejemplo de clase) y los programas separados `huffman_comprimir.c` y `huffman_descomprimir.c`, cuyo archivo comprimido lleva un encabezado (firma `HUF1`, tamaño original y tabla de frecuencias) para reconstruir el árbol.
- Reorganización del repositorio en carpetas.

## Cómo compilar y ejecutar

Desde la raíz del repositorio:

```bash
# Algoritmo genético (acepta una semilla opcional)
gcc -O2 -Wall 04-algoritmo-genetico/ag_rastrigin_32bits.c -o ag -lm
./ag 42

# Huffman en C: ejemplo de clase (crea origen.txt, archivo.bin y recuperado.txt)
gcc -Wall 05-huffman/huffman.c -o huffman
./huffman

# Huffman en C: compresor y descompresor por separado
gcc -O2 05-huffman/huffman_comprimir.c -o huffman_comprimir
gcc -O2 05-huffman/huffman_descomprimir.c -o huffman_descomprimir
./huffman_comprimir 05-huffman/texto_prueba.txt archivo.huf
./huffman_descomprimir archivo.huf recuperado.txt
cmp 05-huffman/texto_prueba.txt recuperado.txt    # sin salida = archivos idénticos

# Estructuras de datos
gcc 02-estructuras-datos/fifo.c -o fifo -Wall -Wextra -std=c11
gcc 02-estructuras-datos/lifo.c -o lifo -Wall -Wextra -std=c11
```

Las instrucciones para compilar y ejecutar la versión en Cython y el benchmark están en [`06-huffman-cython/README.md`](06-huffman-cython/README.md).
