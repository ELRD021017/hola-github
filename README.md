# Hi GitHub

Repositorio de la clase de computación. Aquí guardo los programas que voy haciendo en C, organizados por carpetas.

## Estructura del repositorio

| Carpeta | Contenido |
|---|---|
| `01-introduccion/` | `hola.c`: primer programa en C que imprime un mensaje en pantalla. |
| `02-estructuras-datos/` | `fifo.c` (cola FIFO con listas doblemente ligadas), `lifo.c` (pila LIFO con listas doblemente ligadas) y `hash.c` (tabla hash). |
| `03-algoritmos-numericos/` | `horner_float.c` (evaluación de polinomios con el método de Horner) y `mulx128b.c`. |
| `04-algoritmo-genetico/` | `ag_rastrigin_32bits.c`: algoritmo genético con codificación binaria que minimiza la función de Rastrigin de dos variables. |
| `05-huffman/` | Compresión y descompresión con el algoritmo de Huffman: `huffman.c`, `huffman_comprimir.c` y `huffman_descomprimir.c`. |

## Última actualización

Se añadieron los programas de las tareas más recientes:

- **Algoritmo genético (`04-algoritmo-genetico/`)**: individuos de 64 bits (32 bits para `x` y 32 para `y`), población de 50 individuos, cruza 0.9 y mutación 0.1. Los individuos se ordenan con `qsort` y sobreviven los mejores.
- **Huffman (`05-huffman/`)**:
  - `huffman.c`: traducción a C del ejemplo de clase; crea un texto, lo comprime y lo descomprime.
  - `huffman_comprimir.c` y `huffman_descomprimir.c`: dos programas separados. El archivo comprimido lleva un encabezado (firma `HUF1`, tamaño original y tabla de frecuencias) con el que el descompresor reconstruye el árbol de Huffman.
- Se reorganizó el repositorio en carpetas.

## Cómo compilar y ejecutar

Desde la raíz del repositorio:

```bash
# Algoritmo genético (acepta una semilla opcional)
gcc -O2 -Wall 04-algoritmo-genetico/ag_rastrigin_32bits.c -o ag -lm
./ag 42

# Huffman: ejemplo de clase (crea origen.txt, archivo.bin y recuperado.txt)
gcc -Wall 05-huffman/huffman.c -o huffman
./huffman

# Huffman: compresor y descompresor por separado
gcc -O2 05-huffman/huffman_comprimir.c -o huffman_comprimir
gcc -O2 05-huffman/huffman_descomprimir.c -o huffman_descomprimir
./huffman_comprimir 05-huffman/texto_prueba.txt archivo.huf
./huffman_descomprimir archivo.huf recuperado.txt
cmp 05-huffman/texto_prueba.txt recuperado.txt    # sin salida = archivos idénticos

# Estructuras de datos
gcc 02-estructuras-datos/fifo.c -o fifo -Wall -Wextra -std=c11
gcc 02-estructuras-datos/lifo.c -o lifo -Wall -Wextra -std=c11
```
