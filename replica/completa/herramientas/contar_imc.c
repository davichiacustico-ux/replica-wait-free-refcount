// contar_imc.c — bytes reales que pasan por la controladora de memoria mientras corre una orden.
// Lee los contadores uncore_imc_free_running con perf_event_open, porque este kernel no trae la herramienta perf.
// Cada unidad del contador son 64 bytes (events/data_read.scale). Necesita root.
//   sudo ./contar_imc <intervalo_ms> <orden> [argumentos...]
// Escribe en stderr una línea «IMC,<segundos>,<GB leídos>,<GB escritos>» por intervalo.
#define _GNU_SOURCE
#include <linux/perf_event.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int abrir(int controladora, const char* evento) {
    char ruta[160];
    int tipo;
    unsigned ev, um;
    snprintf(ruta, sizeof ruta, "/sys/bus/event_source/devices/uncore_imc_free_running_%d/type", controladora);
    FILE* f = fopen(ruta, "r");
    if (!f || fscanf(f, "%d", &tipo) != 1) { perror(ruta); exit(3); }
    fclose(f);
    snprintf(ruta, sizeof ruta, "/sys/bus/event_source/devices/uncore_imc_free_running_%d/events/%s", controladora, evento);
    f = fopen(ruta, "r");
    if (!f || fscanf(f, "event=0x%x,umask=0x%x", &ev, &um) != 2) { perror(ruta); exit(3); }
    fclose(f);
    struct perf_event_attr a;
    memset(&a, 0, sizeof a);
    a.type = tipo;
    a.size = sizeof a;
    a.config = ev | (um << 8);
    int fd = syscall(SYS_perf_event_open, &a, -1, 0, -1, 0);
    if (fd < 0) { perror("perf_event_open"); exit(3); }
    return fd;
}

static uint64_t leer(const int* fds) {
    uint64_t total = 0, v;
    for (int i = 0; i < 2; i++) {
        if (read(fds[i], &v, sizeof v) != sizeof v) { perror("read"); exit(3); }
        total += v;
    }
    return total;
}

static double ahora(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int main(int argc, char** argv) {
    if (argc < 3) { fputs("uso: contar_imc intervalo_ms orden [argumentos...]\n", stderr); return 2; }
    long ms = atol(argv[1]);
    int lectura[2] = {abrir(0, "data_read"), abrir(1, "data_read")};
    int escritura[2] = {abrir(0, "data_write"), abrir(1, "data_write")};
    pid_t hijo = fork();
    if (hijo == 0) { execvp(argv[2], argv + 2); perror(argv[2]); _exit(127); }
    uint64_t r0 = leer(lectura), w0 = leer(escritura);
    double inicio = ahora();
    struct timespec paso = {ms / 1000, (ms % 1000) * 1000000};
    int estado = 0;
    while (waitpid(hijo, &estado, WNOHANG) == 0) {
        nanosleep(&paso, NULL);
        uint64_t r1 = leer(lectura), w1 = leer(escritura);
        fprintf(stderr, "IMC,%.3f,%.6f,%.6f\n", ahora() - inicio, (r1 - r0) * 64 / 1e9, (w1 - w0) * 64 / 1e9);
        r0 = r1;
        w0 = w1;
    }
    return WIFEXITED(estado) ? WEXITSTATUS(estado) : 1;
}
