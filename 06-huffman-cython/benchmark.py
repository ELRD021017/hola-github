#!/usr/bin/env python3
"""
Compara Huffman en C (../05-huffman) contra Huffman en Cython (huffman_cy).

Antes de ejecutarlo:
    python setup.py build_ext --inplace      # compila el modulo Cython
    (necesita gcc, que tambien se usa aqui para compilar los programas en C)

Uso:
    python benchmark.py             # tamanos completos (unos 30 MB de datos de prueba)
    python benchmark.py --rapido    # tamanos pequenos, para probar que todo funciona

Que mide:
  * "C"                -> tiempo del programa en C completo (proceso nuevo, lee y escribe archivos)
  * "Cython (proceso)" -> tiempo de `python comprimir_cy.py ...` completo, o sea incluye
                          arrancar el interprete de Python e importar el modulo
  * "Cython (modulo)"  -> tiempo de llamar huffman_cy.comprimir(...) ya importado,
                          que es la comparacion mas justa del algoritmo en si
Todos los tiempos son la MEDIANA de varias repeticiones, en milisegundos.
Tambien comprueba que C y Cython generan archivos IDENTICOS y que cada uno puede
descomprimir lo que comprimio el otro.
"""
import argparse
import hashlib
import os
import random
import shutil
import statistics
import subprocess
import sys
import tempfile
import time
from pathlib import Path

AQUI = Path(__file__).resolve().parent
CARPETA_C = AQUI.parent / "05-huffman"


def sha(ruta):
    return hashlib.sha256(Path(ruta).read_bytes()).hexdigest()


def correr(cmd):
    subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def mediana_ms(funcion, repeticiones):
    tiempos = []
    for _ in range(repeticiones):
        t0 = time.perf_counter()
        funcion()
        tiempos.append((time.perf_counter() - t0) * 1000.0)
    return statistics.median(tiempos)


def compilar_c(destino):
    if shutil.which("gcc") is None:
        sys.exit("No se encontro gcc. Instalalo (en Ubuntu: sudo apt install build-essential).")
    for nombre in ("huffman_comprimir", "huffman_descomprimir"):
        fuente = CARPETA_C / f"{nombre}.c"
        if not fuente.exists():
            sys.exit(f"No se encontro {fuente}. Este script debe estar en 06-huffman-cython/, "
                     f"junto a la carpeta 05-huffman/.")
        subprocess.run(["gcc", "-O2", str(fuente), "-o", str(destino / nombre)], check=True)


def importar_cython():
    sys.path.insert(0, str(AQUI))
    try:
        import huffman_cy
    except ImportError:
        sys.exit("No se encontro el modulo huffman_cy. Compilalo primero con:\n"
                 "  python setup.py build_ext --inplace")
    return huffman_cy


def generar_archivos(carpeta, rapido):
    """Crea los archivos de prueba y devuelve [(nombre, ruta, repeticiones)]."""
    random.seed(2026)
    silabas = ["ba", "ce", "di", "fo", "gu", "la", "me", "ni", "po", "qu",
               "ra", "se", "ti", "un", "ve", "xa", "yo", "zu", "ar", "es"]
    palabras = ["".join(random.choice(silabas) for _ in range(random.randint(1, 4)))
                for _ in range(400)]

    def texto(mb):
        objetivo = int(mb * 1024 * 1024)
        partes, total = [], 0
        while total < objetivo:
            bloque = " ".join(random.choices(palabras, k=20000)) + "\n"
            partes.append(bloque)
            total += len(bloque)
        return "".join(partes).encode()[:objetivo]

    mb_chico, mb_grande, mb_azar = (0.25, 1, 0.25) if rapido else (1, 20, 5)
    archivos = []

    muestra = CARPETA_C / "texto_prueba.txt"
    if muestra.exists():
        ruta = carpeta / "texto_prueba.txt"
        shutil.copy(muestra, ruta)
        archivos.append(("texto_prueba.txt", ruta, 15))

    for nombre, datos, reps in [
        (f"texto_{mb_chico:g}MB", texto(mb_chico), 7),
        (f"texto_{mb_grande:g}MB", texto(mb_grande), 3),
        (f"aleatorio_{mb_azar:g}MB", os.urandom(int(mb_azar * 1024 * 1024)), 3),
    ]:
        ruta = carpeta / (nombre + ".bin")
        ruta.write_bytes(datos)
        archivos.append((nombre, ruta, reps))
    return archivos


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rapido", action="store_true", help="usar archivos pequenos")
    args = ap.parse_args()

    huffman_cy = importar_cython()

    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        bin_c = tmp / "bin"
        bin_c.mkdir()
        compilar_c(bin_c)
        c_comp, c_desc = str(bin_c / "huffman_comprimir"), str(bin_c / "huffman_descomprimir")
        py_comp, py_desc = str(AQUI / "comprimir_cy.py"), str(AQUI / "descomprimir_cy.py")

        archivos = generar_archivos(tmp, args.rapido)
        filas_comp, filas_desc, verificaciones = [], [], []

        for nombre, ruta, reps in archivos:
            tam = ruta.stat().st_size
            huf_c, huf_cy = tmp / "c.huf", tmp / "cy.huf"
            rec_1, rec_2 = tmp / "rec1", tmp / "rec2"

            # ---- Comprimir ----
            t_c = mediana_ms(lambda: correr([c_comp, str(ruta), str(huf_c)]), reps)
            t_cyp = mediana_ms(lambda: correr([sys.executable, py_comp, str(ruta), str(huf_cy)]), reps)
            t_cym = mediana_ms(lambda: huffman_cy.comprimir(str(ruta), str(huf_cy)), reps)
            tam_huf = huf_c.stat().st_size
            filas_comp.append((nombre, tam, tam_huf, t_c, t_cyp, t_cym))

            # ---- Descomprimir (cada uno lee su propio archivo) ----
            t_c = mediana_ms(lambda: correr([c_desc, str(huf_c), str(rec_1)]), reps)
            t_cyp = mediana_ms(lambda: correr([sys.executable, py_desc, str(huf_cy), str(rec_2)]), reps)
            t_cym = mediana_ms(lambda: huffman_cy.descomprimir(str(huf_cy), str(rec_2)), reps)
            filas_desc.append((nombre, tam, tam_huf, t_c, t_cyp, t_cym))

            # ---- Verificaciones cruzadas ----
            identicos = huf_c.read_bytes() == huf_cy.read_bytes()
            correr([c_desc, str(huf_cy), str(rec_1)])        # C lee lo que escribio Cython
            huffman_cy.descomprimir(str(huf_c), str(rec_2))  # Cython lee lo que escribio C
            cruzado = sha(ruta) == sha(rec_1) == sha(rec_2)
            verificaciones.append((nombre, identicos, cruzado))

    def tabla(titulo, filas):
        lineas = [titulo,
                  f"{'Archivo':<20}{'Original':>12}{'Comprimido':>12}{'C':>10}"
                  f"{'Cython(proc)':>14}{'Cython(mod)':>13}{'C/Cy mod':>10}",
                  "-" * 91]
        for nombre, tam, comp, t_c, t_p, t_m in filas:
            lineas.append(f"{nombre:<20}{tam:>12,}{comp:>12,}{t_c:>10.1f}{t_p:>14.1f}{t_m:>13.1f}"
                          f"{(t_c / t_m if t_m else 0):>9.2f}x")
        return "\n".join(lineas)

    salida = [
        tabla("COMPRIMIR (tiempos en ms, mediana)", filas_comp), "",
        tabla("DESCOMPRIMIR (tiempos en ms, mediana)", filas_desc), "",
        "Columna 'C/Cy mod': tiempo de C dividido entre tiempo del modulo Cython. "
        "Menor que 1 = C es mas rapido; mayor que 1 = Cython es mas rapido.", "",
        "VERIFICACION",
    ]
    for nombre, identicos, cruzado in verificaciones:
        salida.append(f"  {nombre:<20} archivos .huf identicos: {'SI' if identicos else 'NO'}   "
                      f"C<->Cython recuperan el original: {'SI' if cruzado else 'NO'}")
    texto_final = "\n".join(salida)
    print(texto_final)
    (AQUI / "resultados_benchmark.txt").write_text(texto_final + "\n", encoding="utf-8")
    print("\nResultados guardados en resultados_benchmark.txt")


if __name__ == "__main__":
    main()
