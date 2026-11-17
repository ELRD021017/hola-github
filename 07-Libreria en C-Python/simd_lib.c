// simd_lib.c
#include <immintrin.h>
#include <stddef.h>
#include <math.h>

/* Reducción horizontal: suma los 8 floats de un __m256 → float */
static inline float hsum256_ps(__m256 v) {
    __m128 lo = _mm256_castps256_ps128(v);
    __m128 hi = _mm256_extractf128_ps(v, 1);
    __m128 s  = _mm_add_ps(lo, hi);
    s = _mm_hadd_ps(s, s);
    s = _mm_hadd_ps(s, s);
    return _mm_cvtss_f32(s);
}

void vec_add(const float* a, const float* b, float* out, size_t n) {
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        _mm256_storeu_ps(out + i, _mm256_add_ps(va, vb));
    }
    for (; i < n; ++i) out[i] = a[i] + b[i];
}

void vec_sub(const float* a, const float* b, float* out, size_t n) {
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        _mm256_storeu_ps(out + i, _mm256_sub_ps(va, vb));
    }
    for (; i < n; ++i) out[i] = a[i] - b[i];
}

void vec_mul(const float* a, const float* b, float* out, size_t n) {
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        _mm256_storeu_ps(out + i, _mm256_mul_ps(va, vb));
    }
    for (; i < n; ++i) out[i] = a[i] * b[i];
}

void vec_scale(const float* a, float s, float* out, size_t n) {
    __m256 vs = _mm256_set1_ps(s);
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        _mm256_storeu_ps(out + i, _mm256_mul_ps(va, vs));
    }
    for (; i < n; ++i) out[i] = a[i] * s;
}

void vec_max(const float* a, const float* b, float* out, size_t n) {
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        _mm256_storeu_ps(out + i, _mm256_max_ps(va, vb));
    }
    for (; i < n; ++i) out[i] = a[i] > b[i] ? a[i] : b[i];
}

void vec_abs(const float* a, float* out, size_t n) {
    // Limpiar el bit de signo con AND-NOT
    __m256 mask = _mm256_set1_ps(-0.0f);
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        _mm256_storeu_ps(out + i, _mm256_andnot_ps(mask, va));
    }
    for (; i < n; ++i) out[i] = a[i] < 0.0f ? -a[i] : a[i];
}

void vec_sqrt(const float* a, float* out, size_t n) {
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        _mm256_storeu_ps(out + i, _mm256_sqrt_ps(va));
    }
    for (; i < n; ++i) out[i] = sqrtf(a[i]);
}

void vec_relu(const float* a, float* out, size_t n) {
    __m256 zero = _mm256_setzero_ps();
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        _mm256_storeu_ps(out + i, _mm256_max_ps(va, zero));
    }
    for (; i < n; ++i) out[i] = a[i] > 0.0f ? a[i] : 0.0f;
}

float vec_dot(const float* a, const float* b, size_t n) {
    __m256 acc = _mm256_setzero_ps();
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        acc = _mm256_add_ps(acc, _mm256_mul_ps(va, vb));
    }
    float r = hsum256_ps(acc);
    for (; i < n; ++i) r += a[i] * b[i];
    return r;
}

float vec_sum(const float* a, size_t n) {
    __m256 acc = _mm256_setzero_ps();
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        acc = _mm256_add_ps(acc, _mm256_loadu_ps(a + i));
    }
    float r = hsum256_ps(acc);
    for (; i < n; ++i) r += a[i];
    return r;
}