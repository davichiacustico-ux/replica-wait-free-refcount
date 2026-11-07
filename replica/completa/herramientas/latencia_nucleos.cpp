// latencia_nucleos.cpp — mide cuánto tarda una línea de caché en viajar entre cada par de núcleos.
//
// Dos hilos fijados a dos núcleos se pasan una «pelota» (un entero atómico): uno escribe, el otro
// espera a verlo y responde. Una ida y vuelta son dos viajes, así que la latencia es la mitad.
// Es lo que cuesta una operación atómica disputada, y por eso sirve para decidir qué grupos de
// núcleos pueden hacer de «socket»: dentro del grupo debe ser barato y entre grupos, caro.
//
// Compilar: g++ -std=c++20 -O2 -pthread latencia_nucleos.cpp -o latencia_nucleos
// Uso:      ./latencia_nucleos [idas_y_vueltas] [repeticiones] > matriz.csv
#include <pthread.h>
#include <sched.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

static void fijar(int cpu) {
    cpu_set_t c;
    CPU_ZERO(&c);
    CPU_SET(cpu, &c);
    pthread_setaffinity_np(pthread_self(), sizeof c, &c);
}

struct alignas(128) Pelota { std::atomic<long> valor{0}; };

static double ida_y_vuelta(int a, int b, long vueltas) {
    Pelota pelota;
    std::thread eco([&] {
        fijar(b);
        for (long i = 1; i <= vueltas; i++) {
            while (pelota.valor.load(std::memory_order_acquire) != 2 * i - 1) {}
            pelota.valor.store(2 * i, std::memory_order_release);
        }
    });
    fijar(a);
    auto t0 = std::chrono::steady_clock::now();
    for (long i = 1; i <= vueltas; i++) {
        pelota.valor.store(2 * i - 1, std::memory_order_release);
        while (pelota.valor.load(std::memory_order_acquire) != 2 * i) {}
    }
    auto t1 = std::chrono::steady_clock::now();
    eco.join();
    return std::chrono::duration<double, std::nano>(t1 - t0).count() / vueltas / 2;
}

int main(int argc, char** argv) {
    long vueltas = argc > 1 ? std::atol(argv[1]) : 200000;
    int reps = argc > 2 ? std::atoi(argv[2]) : 5;
    int n = (int)std::thread::hardware_concurrency();
    std::puts("a,b,ns");
    for (int a = 0; a < n; a++) {
        for (int b = 0; b < n; b++) {
            if (a == b) continue;
            std::vector<double> v;
            for (int r = 0; r < reps; r++) v.push_back(ida_y_vuelta(a, b, vueltas));
            std::sort(v.begin(), v.end());
            std::printf("%d,%d,%.1f\n", a, b, v[v.size() / 2]);
            std::fflush(stdout);
        }
    }
}
