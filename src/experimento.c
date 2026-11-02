#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

typedef enum {
    MODO_MUTEX,
    MODO_ATOMICO
} Modo;

typedef struct {
    Modo modo;
    long incrementos;
} DatosHilo;

static long contador_mutex = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static atomic_long contador_atomico = ATOMIC_VAR_INIT(0);

static void *incrementar(void *arg) {
    DatosHilo *datos = arg;

    for (long i = 0; i < datos->incrementos; i++) {
        if (datos->modo == MODO_MUTEX) {
            pthread_mutex_lock(&mutex);
            contador_mutex++;
            pthread_mutex_unlock(&mutex);
        } else {
            atomic_fetch_add_explicit(
                &contador_atomico, 1, memory_order_relaxed
            );
        }
    }

    return NULL;
}

static int leer_entero(const char *texto, long *valor) {
    char *fin;
    errno = 0;

    long resultado = strtol(texto, &fin, 10);

    if (errno != 0 || *texto == '\0' ||
        *fin != '\0' || resultado <= 0) {
        return 0;
    }

    *valor = resultado;
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr,
            "Uso: %s mutex|atomico NUM_HILOS INCREMENTOS\n",
            argv[0]);
        return EXIT_FAILURE;
    }

    Modo modo;

    if (strcmp(argv[1], "mutex") == 0) {
        modo = MODO_MUTEX;
    } else if (strcmp(argv[1], "atomico") == 0) {
        modo = MODO_ATOMICO;
    } else {
        fprintf(stderr, "Modo invalido.\n");
        return EXIT_FAILURE;
    }

    long numero_hilos;
    long incrementos;

    if (!leer_entero(argv[2], &numero_hilos) ||
        !leer_entero(argv[3], &incrementos) ||
        numero_hilos > 1024 ||
        numero_hilos > INT_MAX ||
        incrementos > LONG_MAX / numero_hilos) {
        fprintf(stderr, "Parametros invalidos.\n");
        return EXIT_FAILURE;
    }

    pthread_t *hilos = calloc(
        (size_t)numero_hilos, sizeof(pthread_t)
    );

    if (hilos == NULL) {
        perror("Error reservando memoria");
        return EXIT_FAILURE;
    }

    DatosHilo datos = {modo, incrementos};

    contador_mutex = 0;
    atomic_store(&contador_atomico, 0);

    struct timespec inicio, fin;

    if (clock_gettime(CLOCK_MONOTONIC, &inicio) != 0) {
        perror("Error al medir el tiempo");
        free(hilos);
        return EXIT_FAILURE;
    }

    long creados = 0;

    for (long i = 0; i < numero_hilos; i++) {
        int error = pthread_create(
            &hilos[i], NULL, incrementar, &datos
        );

        if (error != 0) {
            fprintf(stderr, "Error al crear hilo: %d\n", error);
            break;
        }

        creados++;
    }

    int error_union = 0;

    for (long i = 0; i < creados; i++) {
        int error = pthread_join(hilos[i], NULL);

        if (error != 0) {
            fprintf(stderr, "Error al esperar hilo: %d\n", error);
            error_union = 1;
        }
    }

    if (creados != numero_hilos || error_union) {
        free(hilos);
        pthread_mutex_destroy(&mutex);
        return EXIT_FAILURE;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &fin) != 0) {
        perror("Error al medir el tiempo");
        free(hilos);
        pthread_mutex_destroy(&mutex);
        return EXIT_FAILURE;
    }

    double segundos =
        (double)(fin.tv_sec - inicio.tv_sec) +
        (double)(fin.tv_nsec - inicio.tv_nsec) / 1e9;

    long resultado = (modo == MODO_MUTEX)
        ? contador_mutex
        : atomic_load(&contador_atomico);

    long esperado = numero_hilos * incrementos;

    printf("Mecanismo: %s\n", argv[1]);
    printf("Hilos: %ld\n", numero_hilos);
    printf("Incrementos por hilo: %ld\n", incrementos);
    printf("Resultado esperado: %ld\n", esperado);
    printf("Resultado obtenido: %ld\n", resultado);
    printf("Tiempo total: %.6f segundos\n", segundos);

    free(hilos);
    pthread_mutex_destroy(&mutex);

    return resultado == esperado
        ? EXIT_SUCCESS : EXIT_FAILURE;
}
