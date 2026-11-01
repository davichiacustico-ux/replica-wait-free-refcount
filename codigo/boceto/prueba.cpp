// Prueba de estrés del boceto (explicación, Parte 11): 8 hilos hacen load/store sobre la misma celda.
// Compilar: g++ -std=c++17 -O2 -pthread prueba.cpp -o prueba
#include "atomic_shared.hpp"
#include <cstdio>
#include <random>
#include <thread>
#include <vector>

class Ingenuo {
    std::atomic<Objeto*> ptr{nullptr};
public:
    void store(Objeto* nuevo) { incrementar(nuevo, 1); decrementar(ptr.exchange(nuevo), 1); }
    Objeto* load() { Objeto* p = ptr.load(); std::this_thread::yield(); incrementar(p, 1); return p; }
};

template <class Celda>
void prueba(const char* nombre, int hilos, int ops) {
    objetos_vivos = 0; errores = 0; max_intentos = 0;
    Celda celda;
    Objeto* primero = crear();
    celda.store(primero);
    decrementar(primero, 1);
    std::vector<std::thread> ts;
    for (int t = 0; t < hilos; t++)
        ts.emplace_back([&celda, t, ops] {
            std::mt19937 rng(t + 1);
            for (int i = 0; i < ops; i++) {
                if (rng() % 10 == 0) { Objeto* o = crear(); celda.store(o); decrementar(o, 1); }
                else { Objeto* p = celda.load(); if (p) { comprobar(p); decrementar(p, 1); } }
            }
        });
    for (auto& th : ts) th.join();
    celda.store(nullptr);
    std::printf("%-8s hilos=%d  operaciones=%d\n", nombre, hilos, hilos * ops);
    std::printf("         errores detectados      = %ld\n", errores.load());
    std::printf("         objetos sin liberar     = %ld\n", objetos_vivos.load());
    std::printf("         max. intentos CAS/recarga = %llu (cota P = %llu)\n\n", (unsigned long long)max_intentos.load(), (unsigned long long)P);
}

int main() {
    prueba<AtomicSharedPtr>("paper", 8, 500000);
#ifndef SOLO_PAPER
    prueba<Ingenuo>("ingenuo", 8, 100000);
#endif
}
