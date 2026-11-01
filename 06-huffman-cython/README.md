# Huffman en Cython y comparación con C

Mismo compresor de Huffman que en `../05-huffman`, pero escrito en **Cython**, más un `benchmark.py` que compara el tiempo de los dos.

## Archivos

| Archivo | Para qué sirve |
|---|---|
| `huffman_cy.pyx` | Módulo Cython con las funciones `comprimir` y `descomprimir`. |
| `setup.py` | Compila el módulo con `cythonize`. |
| `comprimir_cy.py`, `descomprimir_cy.py` | Programas de línea de comandos, equivalentes a `huffman_comprimir` y `huffman_descomprimir` de C. |
| `benchmark.py` | Compila los programas en C, mide ambas versiones y comprueba que den el mismo resultado. |

## Formato del archivo

Es el **mismo formato** que el de la versión en C (firma `HUF1`, tamaño original, tabla de frecuencias y bits codificados). Por eso un archivo comprimido con C se puede descomprimir con Cython, y al revés. El árbol se reconstruye a partir de la tabla de frecuencias con un desempate determinista (hojas con id = símbolo, nodos internos con id 256, 257…).

## Cómo compilar y ejecutar

Se necesita `gcc` y los encabezados de Python (en Ubuntu: `sudo apt install build-essential python3-dev python3-venv`).

```bash
cd 06-huffman-cython
python3 -m venv .venv
source .venv/bin/activate
pip install cython setuptools
python setup.py build_ext --inplace

python comprimir_cy.py ../05-huffman/texto_prueba.txt archivo.huf
python descomprimir_cy.py archivo.huf recuperado.txt
cmp ../05-huffman/texto_prueba.txt recuperado.txt    # sin salida = idénticos
```

## Comparación C vs Cython

```bash
python benchmark.py            # tamaños completos
python benchmark.py --rapido   # versión corta para probar
```

Mide tres cosas, todas con la mediana de varias repeticiones:

- **C:** el programa en C completo.
- **Cython (proc):** `python comprimir_cy.py ...` completo, incluyendo arrancar Python e importar el módulo.
- **Cython (mod):** llamar a `huffman_cy.comprimir(...)` con el módulo ya importado, la comparación más justa del algoritmo en sí.

Además comprueba que los dos generan archivos idénticos y que cada uno puede descomprimir lo que comprimió el otro. El resultado se guarda en `resultados_benchmark.txt`.

## Resultados

### Verificación de compatibilidad

Los 4 archivos de prueba producen `.huf` **idénticos byte a byte** en C y en Cython,
y cada implementación descomprime correctamente lo que comprimió la otra:

| Archivo | .huf idénticos | Roundtrip cruzado |
|---|---|---|
| texto_prueba.txt (2.7 KB) | SÍ | SÍ |
| texto_0.25MB | SÍ | SÍ |
| texto_1MB | SÍ | SÍ |
| aleatorio_0.25MB | SÍ | SÍ |

Esto confirma que ambas implementaciones comparten exactamente el mismo formato
de archivo (firma HUF1 + tamaño + tabla de frecuencias + bits empaquetados MSB-first),
que era el objetivo de la tarea.

### Tiempos (mediana de 3 corridas)

COMPRIMIR

| Archivo | Original | Comprimido | C | Cython(proc) | Cython(mod) |
|---|---|---|---|---|---|
| texto_prueba.txt | 2,757 B | 1,695 B | 1.2 ms | 22.7 ms | 0.1 ms |
| texto_0.25MB | 262 KB | 136 KB | 8.0 ms | 25.2 ms | 2.1 ms |
| texto_1MB | 1,049 KB | 546 KB | 29.1 ms | 31.7 ms | 7.8 ms |
| aleatorio_0.25MB | 262 KB | 263 KB | 8.9 ms | 25.5 ms | 2.2 ms |

DESCOMPRIMIR

| Archivo | Comprimido | C | Cython(proc) | Cython(mod) |
|---|---|---|---|---|
| texto_prueba.txt | 1,695 B | 1.2 ms | 22.9 ms | 0.2 ms |
| texto_0.25MB | 136 KB | 9.4 ms | 28.5 ms | 5.3 ms |
| texto_1MB | 546 KB | 35.4 ms | 44.9 ms | 20.7 ms |
| aleatorio_0.25MB | 263 KB | 9.2 ms | 36.8 ms | 13.4 ms |

### Interpretación

- **Tamaño comprimido:** idéntico entre C y Cython en los 4 casos.
  Confirma que la lógica (construcción del árbol, desempate, empaquetado)
  es equivalente.
- **C vs Cython(proc)** es la comparación justa, porque ambos miden
  un proceso completo con su arranque e I/O:
  - Archivos grandes (~1 MB): C gana por ~8-27 %. El arranque de Python
    se diluye y las dos implementaciones quedan casi a la par.
  - Archivos pequeños (~2.7 KB): C gana por ~19× porque el arranque
    de Python pesa ~20 ms, mientras que el binario en C arranca en ~1 ms.
- **Cython(mod)** mide solo el algoritmo con los datos ya en memoria.
  Por eso aparece artificialmente más rápido en compresión: no paga
  lectura ni arranque. No debe interpretarse como "Cython es más rápido".
- **Excepción interesante:** descomprimir `aleatorio_0.25MB` es el único
  caso donde el algoritmo es el cuello de botella real (árboles profundos,
  muchos saltos de puntero). Ahí C gana por **1.45×** (`C/Cy_mod = 0.69`).
  Es la evidencia de que cuando la I/O no domina, C tiene ventaja real.

### Conclusión

El formato de archivo propio funciona: C y Cython son **intercambiables**
y producen archivos idénticos. En rendimiento, **C es más rápido**, pero
la brecha depende del tamaño del archivo: en archivos grandes la diferencia
es modesta (≈10-25 %); en archivos pequeños domina el arranque de Python
(≈20 ms), no el algoritmo. **Cython compilado es competitivo** cuando el
costo del proceso se amortiza, y solo pierde claramente en cargas donde
el recorrido de punteros es intensivo (descompresión de datos aleatorios).
