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
    Registro *Final;    /* centinela final   */
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

/* Verifica si la cola está vacía */
int lista_vacia(Lista *lista) {
    return lista->Inicial->next == lista->Final;
}

/* ================= FIFO / COLA ================= */

/* Encolar: inserta al final de la cola */
int encolar(Lista *lista, const char *nombre, const char *direccion, int edad, const char *pais, const char *club) {
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

/* Desencolar: elimina el primero (después del centinela Inicial) */
int desencolar(Lista *lista, Registro *out) {
    if (lista_vacia(lista)) return 0;

    Registro *N = lista->Inicial->next;

    if (out != NULL) {
        strcpy(out->nombre, N->nombre);
        strcpy(out->direccion, N->direccion);
        out->edad = N->edad;
        strcpy(out->pais, N->pais);
        strcpy(out->club, N->club);
    }

    lista->Inicial->next = N->next;
    N->next->prev = lista->Inicial;

    free(N);
    lista->len--;
    return 1;
}

/* Muestra el contenido de la cola */
void print_list(Lista *lista) {
    if (lista_vacia(lista)) {
        printf("Cola vacia.\n");
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
    Lista fifo;
    Registro temp;

    crear_lista(&fifo);

    printf("===== FIFO (Cola) =====\n\n");

    /* Encolar personas */
    encolar(&fifo, "Eric"   , "Paramaribo 22"   , 23, "Argentina", "Barcelona");
    encolar(&fifo, "Yester" , "Frente al 3B"    , 24, "Brasil"   , "PSG");
    encolar(&fifo, "Gabriel", "Paramaribo 25"   , 23, "EEUU"     , "Ajax");
    encolar(&fifo, "Ernesto", "Cartagena Sur 54", 30, "Cuba"     , "Real Madrid");

    printf("Contenido inicial de la cola:\n");
    print_list(&fifo);
    printf("Tamaño: %d\n\n", fifo.len);

    printf("Desencolando (FIFO: primero en entrar, primero en salir):\n");
    while (desencolar(&fifo, &temp)) {
        printf("Atendido -> %s | %s | %d | %s | %s\n",
       temp.nombre, temp.direccion, temp.edad, temp.pais, temp.club);
    }

    printf("\nTamaño final: %d\n", fifo.len);

    destruir_lista(&fifo);
    return 0;
}
