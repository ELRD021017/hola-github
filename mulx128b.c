#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

/* Unión que permite ver un valor de 128 bits como:
   - Un registro SIMD (__m128i)
   - Dos enteros de 64 bits (parte baja y alta) */
typedef union {
    __m128i m128;
    struct {
        uint64_t lo;   // bits 0-63
        uint64_t hi;   // bits 64-127
    } u64;
} U128;

/* Multiplica dos números de 128 bits y devuelve el producto de 256 bits
   en un arreglo de 4 palabras de 64 bits (de menor a mayor peso). */
void mul128(U128 a, U128 b, uint64_t result[4])
{
    uint64_t a_lo = a.u64.lo;
    uint64_t a_hi = a.u64.hi;
    uint64_t b_lo = b.u64.lo;
    uint64_t b_hi = b.u64.hi;

    unsigned long long hi_lo, hi_hi, lo_lo, lo_hi, mid1, mid2, carry;

    // Productos parciales usando _mulx_u64
    // _mulx_u64(a, b, &high) devuelve la parte baja y escribe la alta en 'high'
    lo_lo   = _mulx_u64(a_lo, b_lo, &hi_lo);   // a_lo * b_lo
    mid1    = _mulx_u64(a_lo, b_hi, &hi_hi);   // a_lo * b_hi
    mid2    = _mulx_u64(a_hi, b_lo, &carry);   // a_hi * b_lo  (carry = parte alta)
    unsigned long long hi_hi_hi;
    unsigned long long hi_hi_lo = _mulx_u64(a_hi, b_hi, &hi_hi_hi); // a_hi * b_hi

    // Acumulación con propagación de acarreos
    // 1) Suma de la parte media
    uint64_t sum_mid = hi_lo + mid1;
    uint64_t carry_mid = (sum_mid < hi_lo) ? 1 : 0;
    sum_mid += mid2;
    carry_mid += (sum_mid < mid2) ? 1 : 0;

    // 2) Suma de la parte alta
    uint64_t sum_high = hi_hi_lo + hi_hi;
    uint64_t carry_high = (sum_high < hi_hi_lo) ? 1 : 0;
    sum_high += carry;
    carry_high += (sum_high < carry) ? 1 : 0;
    sum_high += carry_mid;
    carry_high += (sum_high < carry_mid) ? 1 : 0;

    // Resultado final
    result[0] = lo_lo;
    result[1] = sum_mid;
    result[2] = sum_high;
    result[3] = hi_hi_hi + carry_high;
}

/* Imprime un arreglo de 4 uint64_t como 32 bytes en hexadecimal,
   del byte más significativo al menos significativo. */
void print_bytes(uint64_t r[4])
{
    // Se recorren las palabras de mayor a menor peso
    for (int i = 3; i >= 0; --i) {
        uint64_t v = r[i];
        // Se imprimen los 8 bytes de cada palabra (big-endian dentro de la palabra)
        for (int j = 7; j >= 0; --j) {
            unsigned char byte = (v >> (j * 8)) & 0xFF;
            printf("%02X ", byte);
        }
    }
    printf("\n");
}

int main(void)
{
    // Ejemplo: multiplicar dos números de 128 bits
    // a = 0x0123456789ABCDEF FEDCBA9876543210
    // b = 0x0F1E2D3C4B5A6978 8796A5B4C3D2E1F0
    U128 a = { .u64 = { .lo = 0xFEDCBA9876543210ULL,
                        .hi = 0x0123456789ABCDEFULL } };
    U128 b = { .u64 = { .lo = 0x8796A5B4C3D2E1F0ULL,
                        .hi = 0x0F1E2D3C4B5A6978ULL } };

    uint64_t result[4];
    mul128(a, b, result);

    printf("Resultado (256 bits) en bytes:\n");
    print_bytes(result);

    return 0;
}
