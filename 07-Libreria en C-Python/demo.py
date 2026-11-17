# demo.py
import time
import numpy as np
import simd_lib as simd

# ===============================================================
# 1) Operaciones básicas con dos vectores
# ===============================================================
print("=" * 50)
print("1) OPERACIONES BÁSICAS")
print("=" * 50)

a = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10], dtype=np.float32)
b = np.array([10, 20, 30, 40, 50, 60, 70, 80, 90, 100], dtype=np.float32)

print("a       =", a)
print("b       =", b)
print("a + b   =", simd.add(a, b))
print("a - b   =", simd.sub(a, b))
print("a * b   =", simd.mul(a, b))
print("a * 2   =", simd.scale(a, 2.0))
print()

# ===============================================================
# 2) Operaciones con un solo vector
# ===============================================================
print("=" * 50)
print("2) OPERACIONES CON UN VECTOR")
print("=" * 50)

v = np.array([-4, -1, 0, 1, 4, 9, 16, 25], dtype=np.float32)

print("v       =", v)
print("|v|     =", simd.abs_(v))
print("relu(v) =", simd.relu(v))
print("sqrt(v) =", simd.sqrt(simd.abs_(v)))   # sqrt de valores positivos
print()

# ===============================================================
# 3) Reducciones (devuelven un solo número)
# ===============================================================
print("=" * 50)
print("3) REDUCCIONES")
print("=" * 50)

print("sum(v)       =", simd.sum_(v))
print("dot(a, b)    =", simd.dot(a, b))
print()

# ===============================================================
# 4) Comparación con NumPy
# ===============================================================
print("=" * 50)
print("4) ¿COINCIDE CON NUMPY?")
print("=" * 50)

print("add:    ", np.allclose(simd.add(a, b),     a + b))
print("sub:    ", np.allclose(simd.sub(a, b),     a - b))
print("mul:    ", np.allclose(simd.mul(a, b),     a * b))
print("scale:  ", np.allclose(simd.scale(a, 2),   a * 2))
print("abs:    ", np.allclose(simd.abs_(v),       np.abs(v)))
print("relu:   ", np.allclose(simd.relu(v),       np.maximum(v, 0)))
print("sum:    ", np.isclose(simd.sum_(v),        v.sum()))
print("dot:    ", np.isclose(simd.dot(a, b),      np.dot(a, b)))
print()

# ===============================================================
# 5) Benchmark sencillo (1M elementos)
# ===============================================================
print("=" * 50)
print("5) BENCHMARK: SIMD vs NUMPY (1M elementos)")
print("=" * 50)

N = 1_000_000
x = np.random.randn(N).astype(np.float32)
y = np.random.randn(N).astype(np.float32)

def medir(fn, reps=50):
    fn()  # warm-up
    t0 = time.perf_counter()
    for _ in range(reps):
        fn()
    return (time.perf_counter() - t0) / reps * 1000  # ms

t_simd = medir(lambda: simd.add(x, y))
t_np   = medir(lambda: x + y)

print(f"  add SIMD:  {t_simd:.3f} ms")
print(f"  add NumPy: {t_np:.3f} ms")
print(f"  speedup:   {t_np / t_simd:.2f}x")

t_simd = medir(lambda: simd.dot(x, y))
t_np   = medir(lambda: np.dot(x, y))
print(f"  dot SIMD:  {t_simd:.3f} ms")
print(f"  dot NumPy: {t_np:.3f} ms")
print(f"  speedup:   {t_np / t_simd:.2f}x")