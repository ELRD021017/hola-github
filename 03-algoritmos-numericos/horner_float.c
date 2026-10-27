#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>

typedef unsigned long long bench_t;

static bench_t before;
static bench_t after;

static inline bench_t cycles(void) {
    unsigned int hi, lo;
    __asm__ __volatile__ ("rdtsc\n\t":"=a" (lo), "=d"(hi));
    return ((bench_t) lo) | (((bench_t) hi) << 32);
}

float horner(float X, const float *coef, long size) {
    float ACC = 0.0f;
    long i;
    for (i = 0; i < size; i++) {
        ACC = (ACC + coef[i]) * X;
    }
    return ACC;
}

float horner_intrinsic(float X, const float *coef, long size) {
    const __m128 *ymm0;
    __m128 X128, Y;
    float *R, P;
    long i;

    ymm0 = (const __m128*)coef;

    X128 = _mm_set1_ps(X * X * X * X);
    Y = _mm_set1_ps(0.0f);

    for (i = 0; i < size / 4 - 1; i++) {
        Y = _mm_add_ps(Y, ymm0[i]);
        Y = _mm_mul_ps(Y, X128);
    }

    Y = _mm_add_ps(Y, ymm0[i]);

    R = (float*)&Y;

    P  = R[3] * X;
    P += R[2] * X * X;
    P += R[1] * X * X * X;
    P += R[0] * X * X * X * X;

    return P;
}

int main(void) {
    float X = 1.1f;
    float R;
    int i, num_trails = 100000;

    clock_t t1, t2;

    srand(time(NULL));

    float *coeficientes;
    int j;
    coeficientes = (float*)_mm_malloc(10000 * sizeof(float), 32);

    for (j = 0; j < 1; j++) {
        for (i = 0; i < 10000; i++) {
            coeficientes[i] = (float)(rand() % 1000) / 1000.0f;
            if (i < 10)
                printf("%f\n", coeficientes[i]);
        }
        R = horner(X, coeficientes, 10000);
        printf("%f\n", R);
        R = horner_intrinsic(X, coeficientes, 10000);
        printf("%f\n", R);
    }

    t1 = clock();
    for (j = 0; j < num_trails; j++)
        R = horner(X, coeficientes, 10000);
    t2 = clock();
    float diff = (((float)t2 - (float)t1) / CLOCKS_PER_SEC);
    printf("Time taken: %f\n", diff);

    t1 = clock();
    for (j = 0; j < num_trails; j++)
        R = horner_intrinsic(X, coeficientes, 10000);
    t2 = clock();
    diff = (((float)t2 - (float)t1) / CLOCKS_PER_SEC);
    printf("Time taken: %f\n", diff);

    _mm_free(coeficientes);
    return 0;
}