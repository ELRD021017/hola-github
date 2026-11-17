# simd_lib.py
import ctypes
import numpy as np
import platform
import os

# ---------------------------------------------------------------
# Cargar la biblioteca compartida
# ---------------------------------------------------------------
_here = os.path.dirname(os.path.abspath(__file__))
if platform.system() == "Windows":
    _path = os.path.join(_here, "simd_lib.dll")
else:
    _path = os.path.join(_here, "libsimd.so")

lib = ctypes.CDLL(_path)

# ---------------------------------------------------------------
# Declaración de firmas
# ---------------------------------------------------------------
_fptr = ctypes.POINTER(ctypes.c_float)
_size = ctypes.c_size_t

lib.vec_add.argtypes   = [_fptr, _fptr, _fptr, _size]
lib.vec_sub.argtypes   = [_fptr, _fptr, _fptr, _size]
lib.vec_mul.argtypes   = [_fptr, _fptr, _fptr, _size]
lib.vec_scale.argtypes = [_fptr, ctypes.c_float, _fptr, _size]
lib.vec_max.argtypes   = [_fptr, _fptr, _fptr, _size]
lib.vec_abs.argtypes   = [_fptr, _fptr, _size]
lib.vec_sqrt.argtypes  = [_fptr, _fptr, _size]
lib.vec_relu.argtypes  = [_fptr, _fptr, _size]
lib.vec_dot.argtypes   = [_fptr, _fptr, _size]
lib.vec_sum.argtypes   = [_fptr, _size]

lib.vec_add.restype   = None
lib.vec_sub.restype   = None
lib.vec_mul.restype   = None
lib.vec_scale.restype = None
lib.vec_max.restype   = None
lib.vec_abs.restype   = None
lib.vec_sqrt.restype  = None
lib.vec_relu.restype  = None
lib.vec_dot.restype   = ctypes.c_float
lib.vec_sum.restype   = ctypes.c_float

# ---------------------------------------------------------------
# Helpers internos
# ---------------------------------------------------------------
def _f32(arr):
    """Garantiza float32 + memoria contigua (C-order)."""
    return np.ascontiguousarray(arr, dtype=np.float32)

def _p(arr):
    """Devuelve puntero float* al buffer de un ndarray float32."""
    return arr.ctypes.data_as(_fptr)

# ---------------------------------------------------------------
# API pública (misma firma que NumPy)
# ---------------------------------------------------------------
def add(a, b):
    a, b = _f32(a), _f32(b)
    out = np.empty_like(a)
    lib.vec_add(_p(a), _p(b), _p(out), a.size)
    return out

def sub(a, b):
    a, b = _f32(a), _f32(b)
    out = np.empty_like(a)
    lib.vec_sub(_p(a), _p(b), _p(out), a.size)
    return out

def mul(a, b):
    a, b = _f32(a), _f32(b)
    out = np.empty_like(a)
    lib.vec_mul(_p(a), _p(b), _p(out), a.size)
    return out

def scale(a, s):
    a = _f32(a)
    out = np.empty_like(a)
    lib.vec_scale(_p(a), float(s), _p(out), a.size)
    return out

def maximum(a, b):
    a, b = _f32(a), _f32(b)
    out = np.empty_like(a)
    lib.vec_max(_p(a), _p(b), _p(out), a.size)
    return out

def abs_(a):
    a = _f32(a)
    out = np.empty_like(a)
    lib.vec_abs(_p(a), _p(out), a.size)
    return out

def sqrt(a):
    a = _f32(a)
    out = np.empty_like(a)
    lib.vec_sqrt(_p(a), _p(out), a.size)
    return out

def relu(a):
    a = _f32(a)
    out = np.empty_like(a)
    lib.vec_relu(_p(a), _p(out), a.size)
    return out

def dot(a, b):
    a, b = _f32(a), _f32(b)
    return lib.vec_dot(_p(a), _p(b), a.size)

def sum_(a):
    a = _f32(a)
    return lib.vec_sum(_p(a), a.size)