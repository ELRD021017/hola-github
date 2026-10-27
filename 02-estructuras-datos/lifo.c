#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Registro {
    char nombre[50];
    char direccion[50];
    int edad;
    char pais[50];
    char club[50];
    struct Registro *next;
    struct Registro *prev;
} Registro;

typedef struct {
    Registro *Inicial;  /* centinela inicio */
    Registro *Final;    /* centinela final (tope) */
    int len;
} Lista;

/* Crea un nodo vacío */
Registro *crear() {
    Registro *T = (Registro *)malloc(sizeof(Registro));
    if (T == NULL) {
        printf("Error: no hay memoria.\n");
        exit(1);
    }
    T->next = NULL;
    T->prev = NULL;
    return T;
}

/* Inicializa la lista con centinelas */
void crear_lista(Lista *lista) {
    Registro *N_inicial = crear();
    Registro *N_final   = crear();

    lista->Inicial = N_inicial;
    lista->Final   = N_final;
    lista->len     = 0;

    lista->Inicial->next = N_final;
    lista->Inicial->prev = NULL;

    lista->Final->prev = N_inicial;
    lista->Final->next = NULL;
}

/* Verifica si la pila está vacía */
int lista_vacia(Lista *lista) {
    return lista->Inicial->next == lista->Final;
}

/* ================= LIFO / PILA ================= */

/* Apilar: inserta al final. El nuevo nodo es el tope */
int apilar(Lista *lista, const char *nombre, const char *direccion, int edad, const char *pais, const char *club) {
    Registro *N = crear();

    strncpy(N->nombre, nombre, sizeof(N->nombre) - 1);
    N->nombre[sizeof(N->nombre) - 1] = '\0';

    strncpy(N->direccion, direccion, sizeof(N->direccion) - 1);
    N->direccion[sizeof(N->direccion) - 1] = '\0';

    N->edad = edad;

    strncpy(N->pais, pais, sizeof(N->pais) - 1);
    N->pais[sizeof(N->pais) - 1] = '\0';

    strncpy(N->club, club, sizeof(N->club) - 1);
    N->club[sizeof(N->club) - 1] = '\0';

    N->prev = lista->Final->prev;
    N->next = lista->Final;

    lista->Final->prev->next = N;
    lista->Final->prev = N;

    lista->len++;
    return 1;
}

/* Desapilar: elimina el último insertado (antes del centinela Final) */
int desapilar(Lista *lista, Registro *out) {
    if (lista_vacia(lista)) return 0;

    Registro *N = lista->Final->prev;

    if (out != NULL) {
        strcpy(out->nombre, N->nombre);
        strcpy(out->direccion, N->direccion);
        out->edad = N->edad;
        strcpy(out->pais, N->pais);
        strcpy(out->club, N->club);
    }

    N->prev->next = lista->Final;
    lista->Final->prev = N->prev;

    free(N);
    lista->len--;
    return 1;
}

/* Muestra el contenido de la pila (de base a tope) */
void print_list(Lista *lista) {
    if (lista_vacia(lista)) {
        printf("Pila vacia.\n");
        return;
    }
    Registro *iter;
    for (iter = lista->Inicial->next; iter != lista->Final; iter = iter->next) {
        printf("Nombre: %s | Direccion: %s | Edad: %d | Pais: %s | Club: %s\n",
               iter->nombre, iter->direccion, iter->edad, iter->pais, iter->club);
    }
}

/* Libera la memoria de la lista */
void destruir_lista(Lista *lista) {
    Registro *actual = lista->Inicial;
    while (actual != NULL) {
        Registro *sig = actual->next;
        free(actual);
        actual = sig;
    }
    lista->Inicial = NULL;
    lista->Final = NULL;
    lista->len = 0;
}

/* ================= MAIN ================= */

int main() {
    Lista lifo;
    Registro temp;

    crear_lista(&lifo);

    printf("===== LIFO (Pila) =====\n\n");

    /* Apilar personas */
    apilar(&lifo, "Eric"   , "Paramaribo 22"   ,  23, "Argentina", "Barcelona");
    apilar(&lifo, "Yester" , "Frente al 3B"    ,  24, "Brasil"   , "PSG");
    apilar(&lifo, "Gabriel", "Paramaribo 25"   ,  23, "EEUU"     , "Ajax");
    apilar(&lifo, "Ernesto", "Cartagena Sur 54",  30, "Cuba"     , "Real Madrid");

    printf("Contenido inicial de la pila (base -> tope):\n");
    print_list(&lifo);
    printf("Tamaño: %d\n\n", lifo.len);

    printf("Desapilando (LIFO: ultimo en entrar, primero en salir):\n");
    while (desapilar(&lifo, &temp)) {
        printf("Sacado -> %s | %s | %d | %s | %s\n",
               temp.nombre, temp.direccion, temp.edad, temp.pais, temp.club);
    }

    printf("\nTamaño final: %d\n", lifo.len);

    destruir_lista(&lifo);
    return 0;
}
