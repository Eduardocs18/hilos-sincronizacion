#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_HILOS 4
#define INCREMENTOS 100000

long contador = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *incrementar(void *arg) {
    (void)arg;

    for (long i = 0; i < INCREMENTOS; i++) {
        pthread_mutex_lock(&mutex);
        contador++;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(void) {
    pthread_t hilos[NUM_HILOS];
    struct timespec inicio, fin;

    printf("=== Contador con mutex y medicion ===\n");
    printf("Numero de hilos: %d\n", NUM_HILOS);
    printf("Incrementos por hilo: %d\n", INCREMENTOS);
    printf("Resultado esperado: %d\n",
           NUM_HILOS * INCREMENTOS);

    if (clock_gettime(CLOCK_MONOTONIC, &inicio) != 0) {
        perror("Error al medir el tiempo inicial");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < NUM_HILOS; i++) {
        int error = pthread_create(
            &hilos[i], NULL, incrementar, NULL
        );

        if (error != 0) {
            fprintf(stderr, "Error al crear hilo: %d\n", error);

            for (int j = 0; j < i; j++) {
                pthread_join(hilos[j], NULL);
            }

            pthread_mutex_destroy(&mutex);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < NUM_HILOS; i++) {
        int error = pthread_join(hilos[i], NULL);

        if (error != 0) {
            fprintf(stderr, "Error al esperar hilo: %d\n", error);
            return EXIT_FAILURE;
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &fin) != 0) {
        perror("Error al medir el tiempo final");
        pthread_mutex_destroy(&mutex);
        return EXIT_FAILURE;
    }

    double segundos =
        (double)(fin.tv_sec - inicio.tv_sec) +
        (double)(fin.tv_nsec - inicio.tv_nsec) / 1000000000.0;

    printf("Resultado obtenido: %ld\n", contador);
    printf("Tiempo total: %.6f segundos\n", segundos);

    pthread_mutex_destroy(&mutex);

    return EXIT_SUCCESS;
}
