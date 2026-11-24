// cota_cas.cpp — comprueba la cota del paper sobre el código REAL de los autores:
// increase_weight termina en como mucho P intentos de CAS (§4).
//
// correr_cota_cas.sh genera, a partir del submódulo codigo/autores, una copia instrumentada del
// header con dos cambios: P y C pasan a ser parámetros (COTA_P, COTA_C) y cada vuelta del bucle
// se cuenta con CuentaCAS. Con los valores reales (P = 2^16) la recarga es rarísima, así que
// también se prueba con P pequeño, respetando siempre el requisito C >= 2P del paper.
//
// Uso: ./cota_cas hilos segundos porcentaje_stores
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <utility>
#include <vector>

static std::atomic<long> maximo{0}, llamadas{0}, intentos_totales{0};
static std::atomic<long> histograma[64];

struct CuentaCAS {
    long n = 0;
    ~CuentaCAS() {
        long m = maximo.load(std::memory_order_relaxed);
        while (n > m && !maximo.compare_exchange_weak(m, n, std::memory_order_relaxed)) {}
        llamadas.fetch_add(1, std::memory_order_relaxed);
        intentos_totales.fetch_add(n, std::memory_order_relaxed);
        histograma[n < 63 ? n : 63].fetch_add(1, std::memory_order_relaxed);
    }
};

#include "atomic_shared_ptr_instrumentado.h"

struct Dato {
    long canario = 0x5ca1ab1e;
    ~Dato() { canario = 0; }
};

int main(int argc, char** argv) {
    int hilos = argc > 1 ? std::atoi(argv[1]) : 8;
    double segundos = argc > 2 ? std::atof(argv[2]) : 2.0;
    int pct_stores = argc > 3 ? std::atoi(argv[3]) : 1;
    ivogabe::atomic_shared_ptr<Dato> celda;
    celda.store(ivogabe::shared_ptr<Dato>::make_shared());
    std::atomic<bool> salida{false};
    std::atomic<long> errores{0}, operaciones{0};
    std::vector<std::thread> ts;
    for (int t = 0; t < hilos; t++) {
        ts.emplace_back([&, t] {
            uint64_t x = 88172645463325252ULL + t;
            long ops = 0;
            while (!salida.load(std::memory_order_relaxed)) {
                x ^= x << 13; x ^= x >> 7; x ^= x << 17;
                if ((long)(x % 100) < pct_stores) {
                    celda.store(ivogabe::shared_ptr<Dato>::make_shared());
                } else {
                    auto p = celda.load();
                    if (p && p->canario != 0x5ca1ab1e) errores.fetch_add(1, std::memory_order_relaxed);
                }
                ops++;
            }
            operaciones += ops;
        });
    }
    std::this_thread::sleep_for(std::chrono::duration<double>(segundos));
    salida = true;
    for (auto& h : ts) h.join();
    std::printf("%d,%d,%llu,%llu,%ld,%ld,%ld,%ld,%ld", hilos, pct_stores, (unsigned long long)(COTA_P), (unsigned long long)(COTA_C),
                operaciones.load(), llamadas.load(), intentos_totales.load(), maximo.load(), errores.load());
    for (int i = 1; i <= 8; i++) std::printf(",%ld", histograma[i].load());
    std::puts("");
}
