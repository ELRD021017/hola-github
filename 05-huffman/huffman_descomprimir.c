/*
 * Descompresor Huffman. Lee archivos creados por huffman_comprimir.c o por
 * huffman_comprimir.py (mismo formato; ver huffman_comprimir.c).
 *
 * Compilar: gcc -O2 -Wall huffman_descomprimir.c -o huffman_descomprimir
 * Uso:      ./huffman_descomprimir entrada.huf salida
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    uint64_t freq;
    int izq, der;           /* -1 en las hojas */
} Nodo;

static Nodo nodos[512];

/* Las dos funciones siguientes deben ser IDENTICAS a las del compresor */
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

static uint64_t leer_le(FILE *f, int bytes) {
    uint64_t v = 0;
    for (int i = 0; i < bytes; i++) {
        int c = fgetc(f);
        if (c == EOF) { fprintf(stderr, "Error: cabecera danada.\n"); exit(1); }
        v |= (uint64_t)c << (8 * i);
    }
    return v;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s entrada.huf salida\n", argv[0]);
        return 1;
    }
    FILE *in = fopen(argv[1], "rb");
    if (!in) { perror("entrada"); return 1; }

    /* --- Cabecera --- */
    char firma[4];
    if (fread(firma, 1, 4, in) != 4 || memcmp(firma, "HUF1", 4) != 0) {
        fprintf(stderr, "Error: no es un archivo HUF1 valido.\n");
        return 1;
    }
    uint64_t tam = leer_le(in, 8);
    int n = (int)leer_le(in, 2);
    if (n > 256) { fprintf(stderr, "Error: cabecera danada.\n"); return 1; }

    uint64_t freq[256] = {0};
    for (int i = 0; i < n; i++) {
        int s = (int)leer_le(in, 1);
        freq[s] = leer_le(in, 4);
    }

    FILE *out = fopen(argv[2], "wb");
    if (!out) { perror("salida"); return 1; }

    /* --- Reconstruccion y decodificacion --- */
    uint64_t escritos = 0;
    if (n > 0) {
        int raiz = construir_arbol(freq);
        if (nodos[raiz].izq < 0) {                 /* un solo simbolo distinto */
            for (; escritos < tam; escritos++) fputc(raiz, out);
        } else {
            int nodo = raiz, c;
            while (escritos < tam && (c = fgetc(in)) != EOF) {
                for (int i = 7; i >= 0 && escritos < tam; i--) {
                    nodo = ((c >> i) & 1) ? nodos[nodo].der : nodos[nodo].izq;
                    if (nodos[nodo].izq < 0) {     /* hoja: el indice es el simbolo */
                        fputc(nodo, out);
                        escritos++;
                        nodo = raiz;
                    }
                }
            }
        }
    }
    fclose(in);
    fclose(out);

    if (escritos != tam) {
        fprintf(stderr, "Error: archivo comprimido incompleto o danado.\n");
        return 1;
    }
    printf("%s -> %s: %llu bytes\n", argv[1], argv[2], (unsigned long long)tam);
    return 0;
}
