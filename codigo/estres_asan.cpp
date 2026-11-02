// Prueba de estrés con AddressSanitizer: varios hilos leen y escriben la misma celda compartida.
//   ingenua : load lee el puntero y DESPUÉS suma 1 al contador (dos pasos): carrera read-reclaim.
//   autores : ivogabe::atomic_shared_ptr, el código original del paper (load = un fetch_sub).
// Compilar (Linux/WSL): g++ -std=c++20 -O1 -g -fsanitize=address -pthread estres_asan.cpp -o estres_asan
// Ejecutar:             ./estres_asan ingenua      ./estres_asan autores [hilos] [operaciones_por_hilo]
// ThreadSanitizer:      g++ -std=c++20 -O1 -g -fsanitize=thread -pthread estres_asan.cpp -o estres_tsan
#include <cstdint>
#include <utility>
#include "autores/cpp/atomic_shared_ptr.h"   // submódulo: github.com/ivogabe/wait-free-atomic-shared-pointer
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <thread>
#include <vector>

static int HILOS = 8;
static int OPERACIONES = 200000;   // por hilo; 10 % store, 90 % load + usar + soltar

/* ---------- versión ingenua: contador dentro del objeto, leer y luego apuntarse ---------- */
struct Libro { std::atomic<long> rc{1}; long dato = 42; };
struct Ingenua {
    std::atomic<Libro*> cartel{nullptr};
    static void soltar(Libro* l) { if (l && l->rc.fetch_sub(1) == 1) delete l; }
    void store(Libro* nuevo) { nuevo->rc.fetch_add(1); soltar(cartel.exchange(nuevo)); }  // quien llama conserva su referencia
    Libro* load() {
        Libro* l = cartel.load();          // paso 1: leer el cartel
        std::this_thread::yield();         // ensancha la ventana peligrosa
        if (l) l->rc.fetch_add(1);         // paso 2: apuntarse... ¡l puede estar ya liberado!
        return l;
    }
};

/* ---------- versión de los autores ---------- */
struct Dato { long valor = 42; };

long ejecutar_ingenua() {
    Ingenua celda; Libro* primero = new Libro; celda.store(primero); Ingenua::soltar(primero);
    std::atomic<long> suma{0}; std::vector<std::thread> ts;
    for (int t = 0; t < HILOS; t++) ts.emplace_back([&, t] {
        std::mt19937 rng(t + 1); long s = 0;
        for (int i = 0; i < OPERACIONES; i++) {
            if (rng() % 10 == 0) { Libro* l = new Libro; celda.store(l); Ingenua::soltar(l); }
            else { Libro* l = celda.load(); if (l) { s += l->dato; Ingenua::soltar(l); } }
        }
        suma += s;
    });
    for (auto& th : ts) th.join();
    Ingenua::soltar(celda.cartel.exchange(nullptr));
    return suma;
}

long ejecutar_autores() {
    ivogabe::atomic_shared_ptr<Dato> celda(ivogabe::shared_ptr<Dato>(new Dato));
    std::atomic<long> suma{0}; std::vector<std::thread> ts;
    for (int t = 0; t < HILOS; t++) ts.emplace_back([&, t] {
        std::mt19937 rng(t + 1); long s = 0;
        for (int i = 0; i < OPERACIONES; i++) {
            if (rng() % 10 == 0) celda.store(ivogabe::shared_ptr<Dato>(new Dato));
            else { auto p = celda.load(); if (p) s += p->valor; }
        }
        suma += s;
    });
    for (auto& th : ts) th.join();
    celda.store(ivogabe::shared_ptr<Dato>());
    return suma;
}

int main(int argc, char** argv) {
    const char* v = argc > 1 ? argv[1] : "autores";
    if (argc > 2) HILOS = std::atoi(argv[2]);
    if (argc > 3) OPERACIONES = std::atoi(argv[3]);
    std::printf("version=%s  hilos=%d  operaciones=%d (10 %% store, 90 %% load)\n", v, HILOS, HILOS * OPERACIONES);
    auto t0 = std::chrono::steady_clock::now();
    long s = std::strcmp(v, "ingenua") == 0 ? ejecutar_ingenua() : ejecutar_autores();
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    std::printf("terminado en %.0f ms sin errores de memoria (suma de comprobacion %ld)\n", ms, s);
}
