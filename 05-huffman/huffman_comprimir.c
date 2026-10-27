/*
 * Compresor Huffman (trabaja con bytes, sirve para cualquier archivo).
 *
 * Compilar: gcc -O2 -Wall huffman_comprimir.c -o huffman_comprimir
 * Uso:      ./huffman_comprimir entrada salida.huf
 *
 * FORMATO DEL ARCHIVO COMPRIMIDO (enteros en little-endian)
 *   Offset  Tam.     Campo
 *   0       4        Firma "HUF1"
 *   4       8        Tamano original en bytes (uint64)
 *   12      2        Numero de simbolos distintos n (uint16, 0..256)
 *   14      5 * n    Tabla de frecuencias, ordenada por simbolo:
 *                      1 byte = simbolo, 4 bytes = frecuencia (uint32)
 *   14+5n   ...      Datos codificados (bit mas significativo primero, el
 *                    ultimo byte se rellena con ceros). Si n <= 1 no hay.
 *
 * El arbol se reconstruye a partir de la tabla de frecuencias. El desempate
 * es determinista: hojas con id = simbolo, nodos internos con id = 256, 257...
 * (la misma regla que usan los programas en Python).
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint64_t freq;
    int izq, der;           /* -1 en las hojas */
} Nodo;

static Nodo nodos[512];

/* Saca de la lista el nodo con menor (freq, id) */
static int extraer_menor(int *vivos, int *n) {
    int mejor = 0;
    for (int i = 1; i < *n; i++) {
        int a = vivos[i], b = vivos[mejor];
        if (nodos[a].freq < nodos[b].freq ||
            (nodos[a].freq == nodos[b].freq && a < b))
            mejor = i;
    }
    int idx = vivos[mejor];
    vivos[mejor] = vivos[--(*n)];
    return idx;
}

/* Devuelve el indice de la raiz (-1 si no hay simbolos) */
static int construir_arbol(const uint64_t freq[256]) {
    int vivos[256], nvivos = 0, siguiente = 256;
    for (int s = 0; s < 256; s++) {
        nodos[s].freq = freq[s];
        nodos[s].izq = nodos[s].der = -1;
        if (freq[s] > 0) vivos[nvivos++] = s;
    }
    if (nvivos == 0) return -1;
    while (nvivos > 1) {
        int a = extraer_menor(vivos, &nvivos);
        int b = extraer_menor(vivos, &nvivos);
        nodos[siguiente].freq = nodos[a].freq + nodos[b].freq;
        nodos[siguiente].izq = a;
        nodos[siguiente].der = b;
        vivos[nvivos++] = siguiente++;
    }
    return vivos[0];
}

static void generar_codigos(int nodo, uint64_t codigo, int largo,
                            uint64_t codigos[256], int largos[256]) {
    if (nodos[nodo].izq < 0) {
        codigos[nodo] = codigo;
        largos[nodo] = largo;
        return;
    }
    if (largo >= 63) {
        fprintf(stderr, "Error: codigo demasiado largo.\n");
        exit(1);
    }
    generar_codigos(nodos[nodo].izq, codigo << 1, largo + 1, codigos, largos);
    generar_codigos(nodos[nodo].der, (codigo << 1) | 1, largo + 1, codigos, largos);
}

static void escribir_le(FILE *f, uint64_t v, int bytes) {
    for (int i = 0; i < bytes; i++) fputc((int)((v >> (8 * i)) & 0xFF), f);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s entrada salida.huf\n", argv[0]);
        return 1;
    }
    FILE *in = fopen(argv[1], "rb");
    if (!in) { perror("entrada"); return 1; }

    /* Pasada 1: contar frecuencias */
    uint64_t freq[256] = {0}, total = 0;
    int c;
    while ((c = fgetc(in)) != EOF) { freq[c]++; total++; }

    int nsim = 0;
    for (int s = 0; s < 256; s++) {
        if (freq[s] > 0xFFFFFFFFULL) {
            fprintf(stderr, "Error: archivo demasiado grande.\n");
            return 1;
        }
        if (freq[s] > 0) nsim++;
    }

    FILE *out = fopen(argv[2], "wb");
    if (!out) { perror("salida"); return 1; }

    /* Cabecera */
    fwrite("HUF1", 1, 4, out);
    escribir_le(out, total, 8);
    escribir_le(out, (uint64_t)nsim, 2);
    for (int s = 0; s < 256; s++) {
        if (freq[s] > 0) {
            fputc(s, out);
            escribir_le(out, freq[s], 4);
        }
    }

    /* Pasada 2: datos codificados (solo con 2 o mas simbolos distintos) */
    if (nsim > 1) {
        uint64_t codigos[256] = {0};
        int largos[256] = {0};
        generar_codigos(construir_arbol(freq), 0, 0, codigos, largos);

        rewind(in);
        int cur = 0, nb = 0;
        while ((c = fgetc(in)) != EOF) {
            for (int i = largos[c] - 1; i >= 0; i--) {
                cur = (cur << 1) | (int)((codigos[c] >> i) & 1);
                if (++nb == 8) { fputc(cur, out); cur = 0; nb = 0; }
            }
        }
        if (nb > 0) fputc(cur << (8 - nb), out);
    }

    long tam_salida = ftell(out);
    fclose(in);
    fclose(out);
    printf("%s: %llu bytes -> %s: %ld bytes\n", argv[1],
           (unsigned long long)total, argv[2], tam_salida);
    return 0;
}
