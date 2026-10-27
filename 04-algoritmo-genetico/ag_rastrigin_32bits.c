/*
 * Tarea 5 - Algoritmo genetico con codificacion binaria
 * Minimiza la funcion de Rastrigin de dos variables:
 *   f(x,y) = 20 + (x^2 - 10cos(2*pi*x)) + (y^2 - 10cos(2*pi*y))
 * Dominio: x, y en [-5.12, 5.12]    Minimo global: f(0,0) = 0
 *
 * Individuo: cadena de 64 bits (unsigned long long)
 *   bits 63..32 -> x (32 bits)      bits 31..0 -> y (32 bits)
 *
 * Decodificacion: X = A + (ValorDecimal / (2^L - 1)) * (B - A),  L = 32
 *
 * Compilar: gcc ag_rastrigin_32bits.c -o ag -lm
 * Ejecutar: ./ag [semilla]
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ---- Parametros ---- */
#define N_IND       50      /* numero de individuos           */
#define N_GEN       100     /* numero de generaciones         */
#define P_CRUZA     0.9     /* porcentaje de cruza            */
#define P_MUTACION  0.1     /* porcentaje de mutacion         */

#define L           32      /* bits por variable              */
#define A_LIM       (-5.12)
#define B_LIM       (5.12)
#define PI          3.14159265358979323846

typedef unsigned long long u64;

#define MASCARA_ALTA 0xFFFFFFFF00000000ULL  /* los 32 bits de x */
#define MASCARA_BAJA 0x00000000FFFFFFFFULL  /* los 32 bits de y */

typedef struct {
    u64    xy;      /* cadena binaria de 64 bits */
    double x, y;    /* valores reales decodificados */
    double fx;      /* f(x,y): menor es mejor */
} Individuo;

static double rand01(void) { return rand() / ((double)RAND_MAX + 1.0); }

/* rand() solo da ~15 bits en algunos sistemas: se arma un u64 aleatorio por partes */
static u64 rand64(void) {
    u64 r = 0;
    for (int i = 0; i < 5; i++) r = (r << 15) ^ (u64)(rand() & 0x7FFF);
    return r;
}

/* X = A + (valor decimal / (2^L - 1)) * (B - A) */
static double decodificar(u64 valor) {
    return A_LIM + ((double)valor / 4294967295.0) * (B_LIM - A_LIM);
}

static double rastrigin(double x, double y) {
    return 20.0 + (x*x - 10.0*cos(2.0*PI*x)) + (y*y - 10.0*cos(2.0*PI*y));
}

/* Paso 5: evaluar f(x) */
static void evaluar(Individuo *ind) {
    ind->x  = decodificar((ind->xy & MASCARA_ALTA) >> L);
    ind->y  = decodificar(ind->xy & MASCARA_BAJA);
    ind->fx = rastrigin(ind->x, ind->y);
}

/* Pasos 1-2: individuo aleatorio */
static Individuo nuevo_individuo(void) {
    Individuo ind;
    ind.xy = rand64();
    evaluar(&ind);
    return ind;
}

/* Paso 3: cruza -> mitad de un padre y mitad del otro, con mascaras (AND) */
static void cruzar(u64 p1, u64 p2, u64 *h1, u64 *h2) {
    *h1 = (p1 & MASCARA_ALTA) | (p2 & MASCARA_BAJA);
    *h2 = (p2 & MASCARA_ALTA) | (p1 & MASCARA_BAJA);
}

/* Paso 4: mutacion -> con prob. P_MUTACION se invierte un bit al azar */
static u64 mutar(u64 c) {
    if (rand01() < P_MUTACION)
        c ^= 1ULL << (rand() % 64);   /* mascara con un solo 1, XOR invierte el bit */
    return c;
}

/* Paso 6: orden ascendente por f(x) para qsort */
static int comparar(const void *a, const void *b) {
    double fa = ((const Individuo *)a)->fx, fb = ((const Individuo *)b)->fx;
    return (fa > fb) - (fa < fb);
}

static void imprimir_bits(u64 c) {
    for (int i = 63; i >= 0; i--) {
        putchar(((c >> i) & 1ULL) ? '1' : '0');
        if (i == 32) putchar(' ');   /* separa x | y */
    }
}

int main(int argc, char *argv[]) {
    unsigned int semilla = (argc > 1) ? (unsigned int)atoi(argv[1])
                                      : (unsigned int)time(NULL);
    srand(semilla);

    Individuo pob[2 * N_IND];   /* N_IND padres + N_IND hijos */

    printf("AG - Rastrigin 2D | individuos=%d generaciones=%d cruza=%.2f mutacion=%.2f bits/var=%d semilla=%u\n\n",
           N_IND, N_GEN, P_CRUZA, P_MUTACION, L, semilla);

    for (int i = 0; i < N_IND; i++) pob[i] = nuevo_individuo();
    qsort(pob, N_IND, sizeof(Individuo), comparar);

    printf("%-6s %-12s %-12s %-12s\n", "Gen", "x", "y", "f(x,y)");
    printf("%-6d %-12.6f %-12.6f %-12.6f\n", 0, pob[0].x, pob[0].y, pob[0].fx);

    for (int g = 1; g <= N_GEN; g++) {
        int n = 0;
        while (n < N_IND) {
            /* Paso 3: dos individuos al azar */
            u64 p1 = pob[rand() % N_IND].xy;
            u64 p2 = pob[rand() % N_IND].xy;
            u64 h1 = p1, h2 = p2;
            if (rand01() < P_CRUZA) cruzar(p1, p2, &h1, &h2);

            /* Paso 4: mutacion a hijos */
            pob[N_IND + n++].xy = mutar(h1);
            if (n < N_IND) pob[N_IND + n++].xy = mutar(h2);
        }
        /* Paso 5: evaluar hijos */
        for (int i = N_IND; i < 2 * N_IND; i++) evaluar(&pob[i]);

        /* Paso 6: ordenar padres+hijos con qsort y quedarse con los 50 mejores */
        qsort(pob, 2 * N_IND, sizeof(Individuo), comparar);

        if (g % 10 == 0)
            printf("%-6d %-12.6f %-12.6f %-12.6f\n", g, pob[0].x, pob[0].y, pob[0].fx);
    }

    printf("\nMejor individuo:\n  xy = ");
    imprimir_bits(pob[0].xy);
    printf("\n  x = %.6f\n  y = %.6f\n  f(x,y) = %.6f\n", pob[0].x, pob[0].y, pob[0].fx);
    return 0;
}
