// Parte 4 · Solución
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

// E1
long sumar_con_mutex(int n_hilos, int por_hilo) {
    long total = 0;
    std::mutex m;
    std::vector<std::thread> hs;
    for (int t = 0; t < n_hilos; t++)
        hs.emplace_back([&] {
            for (int i = 0; i < por_hilo; i++) {
                std::lock_guard<std::mutex> g(m);   // toma el candado; lo suelta al salir del bloque
                total++;
            }
        });
    for (auto& h : hs) h.join();
    return total;
}

// E2
std::vector<int> cada_uno_su_casilla(int n) {
    std::vector<int> v(n, -1);
    std::vector<std::thread> hs;
    for (int i = 0; i < n; i++) hs.emplace_back([&v, i] { v[i] = i; });   // i por copia, v por referencia
    for (auto& h : hs) h.join();
    return v;
}

// E3. Bloqueante: si el hilo que tiene el candado se detiene, los demás esperan sin avanzar.
//     Lock-free exigiría que algún hilo siempre avance; wait-free, que cada hilo termine en un
//     número acotado de sus propios pasos. El paper consigue wait-free en load y store.

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
