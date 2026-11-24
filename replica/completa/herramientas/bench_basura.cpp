// bench_basura.cpp — ¿cuántos objetos muertos se quedan ocupando memoria?
//
// El paper dice que su algoritmo no tiene sobrecoste de espacio: cuando un objeto pierde su última
// referencia se libera en el acto. Anderson, en cambio, usa hazard pointers y retrasa la liberación
// del BLOQUE DE CONTROL (el objeto en sí muere al momento; ver dispose() y retire() en su shared_ptr.hpp).
// Por eso se miden dos cosas, mientras la mitad de los hilos escribe objetos nuevos en una celda y la
// otra mitad los lee:
//   vivos    objetos de usuario: cada uno suma 1 al nacer y resta 1 al destruirse
//   bloques  todo lo reservado con new y aún sin liberar: objetos Y bloques de control
//
// Sin liberación diferida, los objetos no pueden pasar de  1 + 2·E + L  (E escritores, L lectores):
// el de la celda, el nuevo y el viejo de cada escritor y el que sostiene cada lector. Cada objeto
// necesita 2 bloques (objeto + control), y se dan 2 de margen por hilo para lo propio de std::thread.
// Lo que supere ese límite es memoria retenida sin que nadie pueda ya usarla.
//
// Se compila junto a all_ptrs.hpp del artefacto (ver construir_extras.sh).
// Uso: ./bench_basura segundos hilos1 hilos2 ...  > basura.csv
#include <malloc.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <thread>
#include <vector>

#include "all_ptrs.hpp"

static std::atomic<long> vivos{0}, bloques{0}, bytes{0};

void* operator new(std::size_t n) {
    void* p = std::malloc(n ? n : 1);
    if (!p) throw std::bad_alloc();
    bloques.fetch_add(1, std::memory_order_relaxed);
    bytes.fetch_add((long)malloc_usable_size(p), std::memory_order_relaxed);
    return p;
}
void operator delete(void* p) noexcept {
    if (!p) return;
    bloques.fetch_sub(1, std::memory_order_relaxed);
    bytes.fetch_sub((long)malloc_usable_size(p), std::memory_order_relaxed);
    std::free(p);
}
void operator delete(void* p, std::size_t) noexcept { operator delete(p); }

struct Objeto {
    long dato[2] = {0, 0};
    Objeto() { vivos.fetch_add(1, std::memory_order_relaxed); }
    ~Objeto() { vivos.fetch_sub(1, std::memory_order_relaxed); }
};

template <template <typename> typename Atomico, template <typename> typename Compartido>
void medir(const char* nombre, int hilos, double segundos) {
    const int escritores = std::max(1, hilos / 2), lectores = hilos - escritores;
    const long limite = 1 + 2L * escritores + lectores;
    vivos = 0;
    const long limite_bloques = 2 * limite + 2L * hilos;
    const long bloques0 = bloques.load(), bytes0 = bytes.load();
    long pico = 0, pico_bloques = 0, pico_bytes = 0, muestras = 0;
    double suma = 0;
    long operaciones = 0;
    {
        Atomico<Objeto> celda;
        celda.store(Compartido<Objeto>(new Objeto));
        std::atomic<bool> salida{false};
        std::atomic<long> total{0};
        std::vector<std::thread> ts;
        for (int t = 0; t < hilos; t++) {
            ts.emplace_back([&, t] {
                long ops = 0;
                if (t < escritores) {
                    while (!salida.load(std::memory_order_relaxed)) { celda.store(Compartido<Objeto>(new Objeto)); ops++; }
                } else {
                    while (!salida.load(std::memory_order_relaxed)) { auto p = celda.load(); ops += p.get() ? 1 : 0; }
                }
                total += ops;
            });
        }
        auto t0 = std::chrono::steady_clock::now();
        while (std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count() < segundos) {
            long v = vivos.load(std::memory_order_relaxed);
            pico = std::max(pico, v);
            pico_bloques = std::max(pico_bloques, bloques.load(std::memory_order_relaxed) - bloques0);
            pico_bytes = std::max(pico_bytes, bytes.load(std::memory_order_relaxed) - bytes0);
            suma += v;
            muestras++;
            usleep(50);
        }
        salida = true;
        for (auto& h : ts) h.join();
        operaciones = total.load();
    }
    std::printf("%s,%d,%d,%d,%ld,%ld,%.1f,%ld,%ld,%ld,%ld,%ld\n", nombre, hilos, escritores, lectores, limite, pico, suma / muestras,
                limite_bloques, pico_bloques, std::max(0L, pico_bloques - limite_bloques), pico_bytes, operaciones);
    std::fflush(stdout);
}

int main(int argc, char** argv) {
    double segundos = argc > 1 ? std::atof(argv[1]) : 2.0;
    std::vector<int> lista;
    for (int i = 2; i < argc; i++) lista.push_back(std::atoi(argv[i]));
    if (lista.empty()) lista = {2, 4, 8, 16, 24};
    std::puts("impl,hilos,escritores,lectores,limite_objetos,pico_objetos,media_objetos,limite_bloques,pico_bloques,exceso_bloques,pico_bytes,operaciones");
    for (int h : lista) {
        medir<ivogabe::atomic_shared_ptr, ivogabe::shared_ptr>("Autores", h, segundos);
        medir<StlAtomicSharedPtr, std::shared_ptr>("Libstdc++", h, segundos);
        medir<jss::atomic_shared_ptr, jss::shared_ptr>("Williams", h, segundos);
        medir<folly::atomic_shared_ptr, std::shared_ptr>("Folly", h, segundos);
        medir<parlay::atomic_shared_ptr, parlay::shared_ptr>("Anderson", h, segundos);
    }
}
