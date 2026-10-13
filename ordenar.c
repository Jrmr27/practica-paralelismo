#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N 16

// Cada vector tiene un tamaño diferente
#define PEQUENO 20000
#define GRANDE  500000

// Registro de cada tarea
typedef struct {
    int hilo;
    double inicio;
    double fin;
} Evento;

Evento eventos[N];

// Comparación de enteros para qsort()
int comparar(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(int argc, char **argv) {

    if (argc != 3) {
        printf("Uso: %s hilos static|dynamic\n", argv[0]);
        return 1;
    }

    int hilos = atoi(argv[1]);
    if (hilos < 1) return 1;

    int *vectores[N];
    int tamanos[N];

    // Crear y rellenar los vectores
    for (int i = 0; i < N; i++) {

        // Uno de cada cuatro es grande
        tamanos[i] = (i % 4 == 0) ? GRANDE : PEQUENO;

        vectores[i] = malloc(tamanos[i] * sizeof(int));
        if (!vectores[i]) return 1;

        for (int j = 0; j < tamanos[i]; j++)
            vectores[i][j] =
                (int)(((unsigned)j * 1103515245u
                      + (unsigned)i * 12345u) & 0x7fffffffu);
    }

    // Configuración de OpenMP
    omp_set_dynamic(0);

    if (strcmp(argv[2], "static") == 0)
        omp_set_schedule(omp_sched_static, 1);
    else if (strcmp(argv[2], "dynamic") == 0)
        omp_set_schedule(omp_sched_dynamic, 1);
    else {
        printf("Planificacion no valida\n");
        return 1;
    }

    double origen = omp_get_wtime();

    // ====================================
    // ORDENACION PARALELA
    // ====================================

    #pragma omp parallel for num_threads(hilos) schedule(runtime)
    for (int i = 0; i < N; i++) {

        eventos[i].hilo = omp_get_thread_num();
        eventos[i].inicio = omp_get_wtime() - origen;

        // Cada hilo ordena un vector completo
        qsort(vectores[i], tamanos[i],
              sizeof(int), comparar);

        eventos[i].fin = omp_get_wtime() - origen;
    }

    double tiempo = omp_get_wtime() - origen;

    // Comprobar que todos están ordenados
    for (int i = 0; i < N; i++) {
        for (int j = 1; j < tamanos[i]; j++) {
            if (vectores[i][j-1] > vectores[i][j]) {
                printf("ERROR en vector %d\n", i);
                return 1;
            }
        }
    }

    // ====================================
    // EXPORTACION PERFETTO
    // ====================================

    FILE *f = fopen("traza.json", "w");
    if (!f) return 1;

    fprintf(f, "{\"traceEvents\":[\n");

    for (int i = 0; i < N; i++) {
        fprintf(f,
            "%s{\"name\":\"Vector %d\","
            "\"ph\":\"X\","
            "\"pid\":1000,\"tid\":%d,"
            "\"ts\":%.3f,\"dur\":%.3f,"
            "\"args\":{\"elementos\":%d}}",
            i ? ",\n" : "",
            i,
            eventos[i].hilo + 100,
            eventos[i].inicio * 1e6,
            (eventos[i].fin - eventos[i].inicio) * 1e6,
            tamanos[i]);
    }

    fprintf(f, "\n]}\n");
    fclose(f);

    // Resultados
    printf("\nTiempo total: %.4f s\n", tiempo);
    printf("\nVector  Hilo  Elementos   Tiempo (ms)\n");

    for (int i = 0; i < N; i++) {
        printf("%6d  %4d  %9d   %10.3f\n",
               i,
               eventos[i].hilo,
               tamanos[i],
               (eventos[i].fin - eventos[i].inicio) * 1000);
    }

    printf("\nTraza generada: traza.json\n");

    for (int i = 0; i < N; i++)
        free(vectores[i]);

    return 0;
}
