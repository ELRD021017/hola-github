# cython: language_level=3, boundscheck=False, wraparound=False, cdivision=True
"""
Huffman en Cython (compresion y descompresion de cualquier archivo).

Usa EXACTAMENTE el mismo formato de archivo que los programas en C de la
carpeta 05-huffman (firma "HUF1", tamano original, tabla de frecuencias y
bits codificados), asi que un archivo comprimido con C se puede descomprimir
con este modulo y al reves.

Formato (enteros en little-endian):
    0   4      firma "HUF1"
    4   8      tamano original (uint64)
    12  2      numero de simbolos distintos n (uint16)
    14  5*n    tabla: 1 byte simbolo + 4 bytes frecuencia (uint32), por simbolo
    ... resto  bits codificados (bit mas significativo primero, relleno con 0)
               (si n <= 1 no hay bits codificados)

El arbol se reconstruye desde la tabla de frecuencias. Desempate determinista:
hojas con id = simbolo (0..255) y nodos internos con id = 256, 257, ...

Las partes lentas (contar, construir el arbol, codificar y decodificar bit a
bit) usan tipos de C (cdef) y arreglos de C, por eso Cython las compila a
codigo C puro.
"""
import struct

FIRMA = b"HUF1"

# ---- Arreglos globales de C -------------------------------------------------
# Nodos del arbol: hojas = indices 0..255, nodos internos = 256..511
cdef int NODO_IZQ[512]
cdef int NODO_DER[512]
cdef unsigned long long NODO_FREQ[512]
cdef unsigned long long FREQ[256]
cdef unsigned long long CODIGOS[256]
cdef int LARGOS[256]


cdef int _construir_arbol():
    """Construye el arbol desde FREQ. Devuelve la raiz (-1 si no hay simbolos)."""
    cdef int vivos[256]
    cdef int nvivos = 0
    cdef int siguiente = 256
    cdef int s, i, paso, mejor, idx, a, b

    for s in range(256):
        NODO_FREQ[s] = FREQ[s]
        NODO_IZQ[s] = -1
        NODO_DER[s] = -1
        if FREQ[s] > 0:
            vivos[nvivos] = s
            nvivos += 1

    if nvivos == 0:
        return -1

    while nvivos > 1:
        a = -1
        b = -1
        # sacar los dos nodos con menor (frecuencia, id)
        for paso in range(2):
            mejor = 0
            for i in range(1, nvivos):
                if NODO_FREQ[vivos[i]] < NODO_FREQ[vivos[mejor]]:
                    mejor = i
                elif NODO_FREQ[vivos[i]] == NODO_FREQ[vivos[mejor]] and vivos[i] < vivos[mejor]:
                    mejor = i
            idx = vivos[mejor]
            nvivos -= 1
            vivos[mejor] = vivos[nvivos]
            if paso == 0:
                a = idx
            else:
                b = idx
        NODO_FREQ[siguiente] = NODO_FREQ[a] + NODO_FREQ[b]
        NODO_IZQ[siguiente] = a
        NODO_DER[siguiente] = b
        vivos[nvivos] = siguiente
        nvivos += 1
        siguiente += 1

    return vivos[0]


cdef int _generar_codigos(int nodo, unsigned long long codigo, int largo):
    """Llena CODIGOS y LARGOS recorriendo el arbol. Devuelve -1 si un codigo pasa de 56 bits."""
    if NODO_IZQ[nodo] < 0:
        CODIGOS[nodo] = codigo
        LARGOS[nodo] = largo
        return 0
    if largo >= 56:
        return -1
    if _generar_codigos(NODO_IZQ[nodo], codigo << 1, largo + 1) < 0:
        return -1
    return _generar_codigos(NODO_DER[nodo], (codigo << 1) | 1, largo + 1)


def comprimir(str ruta_entrada, str ruta_salida):
    """Comprime ruta_entrada en ruta_salida. Devuelve (tam_original, tam_comprimido)."""
    cdef bytes contenido
    cdef const unsigned char[::1] datos
    cdef unsigned char[::1] salida
    cdef bytearray cabecera
    cdef Py_ssize_t n, i
    cdef Py_ssize_t pos = 0
    cdef Py_ssize_t nbytes
    cdef unsigned long long total_bits = 0
    cdef unsigned long long acc = 0
    cdef unsigned long long uno = 1
    cdef int s, c, raiz
    cdef int nsim = 0
    cdef int nbits = 0

    with open(ruta_entrada, "rb") as f:
        contenido = f.read()
    n = len(contenido)
    if n > 0:
        datos = contenido

    # Paso 1: contar frecuencias
    for s in range(256):
        FREQ[s] = 0
    for i in range(n):
        FREQ[datos[i]] += 1
    for s in range(256):
        if FREQ[s] > 0xFFFFFFFF:
            raise ValueError("archivo demasiado grande (frecuencia mayor a 4 GiB)")
        if FREQ[s] > 0:
            nsim += 1

    # Paso 2: cabecera
    cabecera = bytearray(FIRMA)
    cabecera += struct.pack("<QH", n, nsim)
    for s in range(256):
        if FREQ[s] > 0:
            cabecera += struct.pack("<BI", s, FREQ[s])

    # Paso 3: arbol, codigos y bits (solo con 2 o mas simbolos distintos)
    if nsim > 1:
        raiz = _construir_arbol()
        if _generar_codigos(raiz, 0, 0) < 0:
            raise ValueError("codigo de Huffman demasiado largo")
        for s in range(256):
            if FREQ[s] > 0:
                total_bits += FREQ[s] * LARGOS[s]
        nbytes = (total_bits + 7) // 8
        salida = bytearray(nbytes)

        for i in range(n):
            c = datos[i]
            acc = (acc << LARGOS[c]) | CODIGOS[c]
            nbits += LARGOS[c]
            while nbits >= 8:
                nbits -= 8
                salida[pos] = (acc >> nbits) & 0xFF
                pos += 1
            acc &= (uno << nbits) - 1
        if nbits > 0:
            salida[pos] = (acc << (8 - nbits)) & 0xFF
            pos += 1

    with open(ruta_salida, "wb") as f:
        f.write(cabecera)
        if nsim > 1:
            f.write(salida)

    return n, len(cabecera) + pos


def descomprimir(str ruta_entrada, str ruta_salida):
    """Descomprime ruta_entrada en ruta_salida. Devuelve el tamano recuperado."""
    cdef bytes contenido
    cdef const unsigned char[::1] datos
    cdef unsigned char[::1] salida
    cdef Py_ssize_t total, i, pos
    cdef unsigned long long tam
    cdef unsigned long long limite
    cdef unsigned long long escritos = 0
    cdef unsigned long long freq_s
    cdef int n, s, b, byte, nodo, raiz

    with open(ruta_entrada, "rb") as f:
        contenido = f.read()
    total = len(contenido)

    # Cabecera
    if total < 14 or contenido[:4] != FIRMA:
        raise ValueError("no es un archivo HUF1 valido")
    tam, n = struct.unpack_from("<QH", contenido, 4)
    pos = 14
    if n > 256 or total < pos + 5 * n:
        raise ValueError("cabecera danada")

    for s in range(256):
        FREQ[s] = 0
    for i in range(n):
        s, freq_s = struct.unpack_from("<BI", contenido, pos)
        FREQ[s] = freq_s
        pos += 5

    # Casos sin datos codificados
    if n == 0 or tam == 0:
        if tam != 0:
            raise ValueError("cabecera danada")
        with open(ruta_salida, "wb") as f:
            pass
        return 0

    raiz = _construir_arbol()
    if NODO_IZQ[raiz] < 0:                      # un solo simbolo distinto
        with open(ruta_salida, "wb") as f:
            f.write(bytes([raiz]) * tam)
        return tam

    limite = 8 * (total - pos)                  # cada simbolo ocupa al menos 1 bit
    if tam > limite:
        raise ValueError("archivo comprimido incompleto o danado")

    datos = contenido
    salida = bytearray(tam)

    # Decodificar: bajar por el arbol bit a bit (0 = izquierda, 1 = derecha)
    nodo = raiz
    i = pos
    while escritos < tam and i < total:
        byte = datos[i]
        i += 1
        for b in range(7, -1, -1):
            if (byte >> b) & 1:
                nodo = NODO_DER[nodo]
            else:
                nodo = NODO_IZQ[nodo]
            if NODO_IZQ[nodo] < 0:              # hoja: el indice es el simbolo
                salida[escritos] = nodo
                escritos += 1
                nodo = raiz
                if escritos == tam:
                    break

    if escritos != tam:
        raise ValueError("archivo comprimido incompleto o danado")

    with open(ruta_salida, "wb") as f:
        f.write(salida)
    return tam
