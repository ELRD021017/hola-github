#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define TABLE_SIZE 100

typedef struct {
    char nombre[50];
    int  edad;
    char telefono[20];
    char ciudad[50];
} Persona;

/* Nodo de la lista enlazada */
typedef struct Node {
    char    *key;      /* La clave: cédula (string) */
    Persona *value;    /* Puntero a la persona */
    struct Node *next;
} Node;

/* Tabla hash */
typedef struct {
    Node *buckets[TABLE_SIZE];
} HashTable;

/* =========================================================
 *  FUNCIÓN HASH (sobre la clave string)
 * ========================================================= */
static unsigned int hash_function(const char *key) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash * 31u + (unsigned char)*key) % TABLE_SIZE;
        key++;
    }
    return hash;
}

/* =========================================================
 *  CREAR / LIBERAR LA TABLA
 * ========================================================= */
HashTable* create_hash_table(void) {
    HashTable *table = (HashTable *)malloc(sizeof(HashTable));
    if (!table) return NULL;
    for (int i = 0; i < TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    return table;
}

void free_hash_table(HashTable *table) {
    if (!table) return;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node *current = table->buckets[i];
        while (current) {
            Node *temp = current;
            current = current->next;
            free(temp->key);
            free(temp->value);   /* liberamos también la persona */
            free(temp);
        }
    }
    free(table);
}

/* =========================================================
 *  INSERTAR / ACTUALIZAR  (key = cédula, value = Persona)
 * ========================================================= */
bool insert(HashTable *table, const char *cedula, const Persona *p) {
    if (!table || !cedula || !p) return false;

    unsigned int index = hash_function(cedula);
    Node *current = table->buckets[index];

    /* Si la cédula ya existe, actualizamos los datos */
    while (current) {
        if (strcmp(current->key, cedula) == 0) {
            *(current->value) = *p;   /* copia superficial del struct */
            return true;
        }
        current = current->next;
    }

    /* Si no existe, creamos un nuevo nodo */
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) return false;

    new_node->key = strdup(cedula);    /* copia de la cédula */
    new_node->value = (Persona *)malloc(sizeof(Persona));
    if (!new_node->key || !new_node->value) {
        free(new_node->key);
        free(new_node->value);
        free(new_node);
        return false;
    }
    *(new_node->value) = *p;

    new_node->next = table->buckets[index];
    table->buckets[index] = new_node;
    return true;
}

/* =========================================================
 *  BUSCAR POR CÉDULA
 * ========================================================= */
Persona* get(HashTable *table, const char *cedula) {
    if (!table || !cedula) return NULL;
    unsigned int index = hash_function(cedula);
    Node *current = table->buckets[index];

    while (current) {
        if (strcmp(current->key, cedula) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;   /* No encontrado */
}

/* =========================================================
 *  ELIMINAR POR CÉDULA
 * ========================================================= */
bool remove_key(HashTable *table, const char *cedula) {
    if (!table || !cedula) return false;
    unsigned int index = hash_function(cedula);
    Node *current = table->buckets[index];
    Node *prev = NULL;

    while (current) {
        if (strcmp(current->key, cedula) == 0) {
            if (prev) prev->next = current->next;
            else table->buckets[index] = current->next;
            free(current->key);
            free(current->value);
            free(current);
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

/* =========================================================
 *  IMPRIMIR UNA PERSONA
 * ========================================================= */
void print_persona(const char *cedula, const Persona *p) {
    if (!p) {
        printf("  Cedula %s -> (no encontrada)\n", cedula);
        return;
    }
    printf("  Cedula   : %s\n", cedula);
    printf("  Nombre   : %s\n", p->nombre);
    printf("  Edad     : %d\n", p->edad);
    printf("  Telefono : %s\n", p->telefono);
    printf("  Ciudad   : %s\n", p->ciudad);
    printf("  ------------------------------\n");
}

/* =========================================================
 *  MAIN 
 * ========================================================= */
int main(void) {
    HashTable *registro = create_hash_table();
    if (!registro) {
        fprintf(stderr, "Error creando la tabla\n");
        return 1;
    }

    /* =========================================================
     *  1) La clave es la cédula (string), el valor es el struct.
     * ========================================================= */
    Persona p1 = { "Eric Luis", 23, "5624981300", "CDMX" };
    Persona p2 = { "Yester",    24, "5376477837", "Guadalajara" };
    Persona p3 = { "Gabriel",   23, "5625423771", "Monterrey" };
    Persona p4 = { "Ernesto",   30, "5875236941", "Puebla" };
    Persona p5 = { "Malena",    24, "5433259875", "Baja California" };
    Persona p6 = { "Fabian",    21, "5468223548", "Chiapa" };
    Persona p7 = { "Alhondra",  21, "5454453455", "Chiapa" };
    Persona p8 = { "Freddy",    35, "5468435458", "Tijuana" };
    Persona p9 = { "Felix",     24, "5124442577", "Merida" };

    insert(registro, "001-1234567-8", &p1);
    insert(registro, "002-2345678-9", &p2);
    insert(registro, "003-3456789-0", &p3);
    insert(registro, "004-4567890-1", &p4);
    insert(registro, "005-5678901-2", &p5);
    insert(registro, "006-6789012-3", &p6);
    insert(registro, "007-7890123-4", &p7);
    insert(registro, "008-8901234-5", &p8);
    insert(registro, "009-9012345-6", &p9);

    /* =========================================================
     *  2) BUSCAR Y MOSTRAR
     * ========================================================= */
    printf("===== BUSQUEDA POR CEDULA =====\n");
    const char *cedulas[] = {
        "001-1234567-8",
        "002-2345678-9",
        "003-3456789-0",
        "004-4567890-1",
        "005-5678901-2",
        "006-6789012-3",
        "007-7890123-4",
        "008-8901234-5",
        "009-9012345-6",
    };
    
    int n = sizeof(cedulas) / sizeof(cedulas[0]);
    for (int i = 0; i < n; i++) {
        Persona *p = get(registro, cedulas[i]);
        print_persona(cedulas[i], p);
    }

    /* =========================================================
     *  3) ELIMINAR A ALGUIEN
     * ========================================================= */
    printf("===== ELIMINANDO A 003-3456789-0 (Gabriel) =====\n");
    if (remove_key(registro, "003-3456789-0")) {
        printf("  Eliminado correctamente.\n");
    } else {
        printf("  No se pudo eliminar (no existia).\n");
    }

    printf("\n===== BUSQUEDA DESPUES DE ELIMINAR =====\n");
    Persona *p = get(registro, "003-3456789-0");
    print_persona("003-3456789-0", p);

    /* =========================================================
     *  4) LIBERAR MEMORIA
     * ========================================================= */
    free_hash_table(registro);
    return 0;
}
