// Parte 4 · Ejercicios mínimos de hilos
// Completa los TODO y ejecuta: cada comprobación debe decir [OK].
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

// E1. Lanza n_hilos hilos; cada uno suma 1 a un contador compartido por_hilo veces, protegiéndolo
//     con un std::mutex. Espera a todos (join) y devuelve el total.
long sumar_con_mutex(int n_hilos, int por_hilo) {
    long total = 0;
    // TODO
    return total;
}

// E2. Lanza n hilos; el hilo i escribe i en v[i]. No hace falta candado: cada hilo escribe
//     en una casilla distinta (no comparten datos, no hay carrera).
std::vector<int> cada_uno_su_casilla(int n) {
    std::vector<int> v(n, -1);
    // TODO
    return v;
}

// E3. (para pensar) En E1, ¿qué garantía de progreso tiene el programa si el sistema operativo
//     pausa al hilo que tiene el candado? ¿Bloqueante, lock-free o wait-free?

static int fallos = 0;
static void revisar(bool ok, const char* que) { std::printf("%s %s\n", ok ? "[OK]   " : "[FALTA]", que); if (!ok) fallos++; }

int main() {
    revisar(sumar_con_mutex(4, 100000) == 400000, "E1: 4 hilos x 100000 = 400000 exactos");
    std::vector<int> v = cada_uno_su_casilla(8);
    bool bien = true; for (int i = 0; i < 8; i++) bien = bien && v[i] == i;
    revisar(bien, "E2: v = 0, 1, 2, ..., 7");
    std::printf(fallos ? "\nTe faltan %d.\n" : "\nTodo correcto.\n", fallos);
    return 0;
}
