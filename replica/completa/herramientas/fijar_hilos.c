// fijar_hilos.c — fija cada hilo a un núcleo concreto SIN tocar el binario del paper.
//
// Se carga con LD_PRELOAD. Intercepta pthread_create y le pone a cada hilo nuevo la afinidad
// que le toque según la lista de la variable de entorno FIJAR_CPUS. Así los binarios oficiales
// siguen siendo exactamente los del artefacto: solo cambia en qué núcleo cae cada hilo.
//
//   FIJAR_CPUS="8,12,16,20"  hilo 0 -> CPU 8, hilo 1 -> CPU 12, ... (al acabarse la lista, vuelve a empezar)
//   FIJAR_PRINCIPAL=N        CPU del hilo principal; -1 lo deja suelto; "lista" lo trata como el primer hilo
//   FIJAR_REGISTRO=archivo   anota "hilo N -> CPU M" para comprobar que el reparto fue el pedido
//
// Compilar: gcc -O2 -shared -fPIC fijar_hilos.c -o fijar_hilos.so -ldl -pthread
#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CPUS 256

static int cpus[MAX_CPUS];
static int num_cpus = 0;
static atomic_int siguiente = 0;
static int (*real_pthread_create)(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*) = NULL;

static void anotar(int hilo, int cpu) {
    const char* ruta = getenv("FIJAR_REGISTRO");
    if (!ruta) return;
    FILE* f = fopen(ruta, "a");
    if (!f) return;
    fprintf(f, "hilo %d -> CPU %d\n", hilo, cpu);
    fclose(f);
}

__attribute__((constructor)) static void iniciar(void) {
    real_pthread_create = dlsym(RTLD_NEXT, "pthread_create");
    const char* lista = getenv("FIJAR_CPUS");
    if (!lista || !*lista) return;
    char copia[4096];
    strncpy(copia, lista, sizeof copia - 1);
    copia[sizeof copia - 1] = 0;
    for (char* t = strtok(copia, ","); t && num_cpus < MAX_CPUS; t = strtok(NULL, ","))
        cpus[num_cpus++] = atoi(t);

    const char* principal = getenv("FIJAR_PRINCIPAL");
    if (!principal) return;
    int cpu = -1;
    if (strcmp(principal, "lista") == 0) cpu = cpus[atomic_fetch_add(&siguiente, 1) % num_cpus];
    else cpu = atoi(principal);
    if (cpu >= 0) {
        cpu_set_t c;
        CPU_ZERO(&c);
        CPU_SET(cpu, &c);
        sched_setaffinity(0, sizeof c, &c);
        anotar(-1, cpu);
    }
}

int pthread_create(pthread_t* hilo, const pthread_attr_t* attr, void* (*rutina)(void*), void* arg) {
    if (num_cpus == 0) return real_pthread_create(hilo, attr, rutina, arg);
    int n = atomic_fetch_add(&siguiente, 1);
    int cpu = cpus[n % num_cpus];
    cpu_set_t c;
    CPU_ZERO(&c);
    CPU_SET(cpu, &c);

    pthread_attr_t propio;
    if (attr) {
        // se copian los atributos del llamante: el hilo conserva su pila y su estado
        pthread_attr_init(&propio);
        size_t tam;
        void* pila;
        int desligado;
        if (pthread_attr_getstack(attr, &pila, &tam) == 0 && pila) pthread_attr_setstack(&propio, pila, tam);
        else if (pthread_attr_getstacksize(attr, &tam) == 0) pthread_attr_setstacksize(&propio, tam);
        if (pthread_attr_getdetachstate(attr, &desligado) == 0) pthread_attr_setdetachstate(&propio, desligado);
    } else {
        pthread_attr_init(&propio);
    }
    pthread_attr_setaffinity_np(&propio, sizeof c, &c);
    int r = real_pthread_create(hilo, &propio, rutina, arg);
    pthread_attr_destroy(&propio);
    anotar(n, cpu);
    return r;
}
