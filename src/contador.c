#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_HILOS 4
#define INCREMENTOS 100000

long contador = 0;

void *incrementar(void *arg) {
    (void)arg;

    for (long i = 0; i < INCREMENTOS; i++) {
        contador++;
    }

    return NULL;
}

int main(void) {
    pthread_t hilos[NUM_HILOS];

    printf("=== Contador compartido ===\n");
    printf("Numero de hilos: %d\n", NUM_HILOS);
    printf("Incrementos por hilo: %d\n", INCREMENTOS);
    printf("Resultado esperado: %d\n",
           NUM_HILOS * INCREMENTOS);

    for (int i = 0; i < NUM_HILOS; i++) {
        int error = pthread_create(
            &hilos[i], NULL, incrementar, NULL
        );

        if (error != 0) {
            fprintf(stderr,
                    "Error al crear el hilo: %d\n", error);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < NUM_HILOS; i++) {
        int error = pthread_join(hilos[i], NULL);

        if (error != 0) {
            fprintf(stderr,
                    "Error al esperar el hilo: %d\n", error);
            return EXIT_FAILURE;
        }
    }

    printf("Resultado obtenido: %ld\n", contador);

    return EXIT_SUCCESS;
}
