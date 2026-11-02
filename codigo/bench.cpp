// bench.cpp — mini-reproducción de los experimentos del paper (sección 6) en un solo equipo.
//   rendimiento: n pilas concurrentes (pila de Treiber hecha con atomic shared pointers) y u % de actualizaciones;
//                una actualización saca un valor de una pila aleatoria y lo mete en otra; el resto lee la cima.
//   latencia:    p hilos; el primero mide load, store (sin liberar) y store & reclaim; los demás hacen store sin parar.
// Implementaciones: Autores  = ivogabe::atomic_shared_ptr (código original del paper, wait-free)
//                   Libstdc++ = std::atomic<std::shared_ptr<T>> de GCC (con candado interno)
//                   Williams  = jss::atomic_shared_ptr (split reference count, lock-free; rival del paper)
//                   Mutex     = std::shared_ptr protegido con std::mutex (referencia sencilla)
// Compilar (Linux/WSL): g++ -std=c++20 -O2 -pthread bench.cpp -o bench
// Uso: ./bench rendimiento [segundos] > rendimiento.csv     ./bench latencia [muestras] > latencia.csv
#include <cstdint>
#include <utility>
#include "autores/cpp/atomic_shared_ptr.h"   // submódulo: github.com/ivogabe/wait-free-atomic-shared-pointer
#include "williams/atomic_shared_ptr"        // submódulo: github.com/anthonywilliams/atomic_shared_ptr (BSD)
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <x86intrin.h>

/* ---------- las cuatro implementaciones con la misma interfaz ---------- */
struct Autores {
    static constexpr const char* nombre = "Autores";
    template <class T> using SP = ivogabe::shared_ptr<T>;
    template <class T> using ASP = ivogabe::atomic_shared_ptr<T>;
    template <class T, class... A> static SP<T> crear(A&&... a) { return SP<T>(new T{std::forward<A>(a)...}); }
    template <class T> static SP<T> load(ASP<T>& c) { return c.load(); }
    template <class T> static void store(ASP<T>& c, SP<T> p) { c.store(std::move(p)); }
    template <class T> static bool cas(ASP<T>& c, const SP<T>& esperado, const SP<T>& nuevo) { return c.compare_and_set(esperado, nuevo); }
};
struct Libstdcpp {
    static constexpr const char* nombre = "Libstdc++";
    template <class T> using SP = std::shared_ptr<T>;
    template <class T> using ASP = std::atomic<std::shared_ptr<T>>;
    template <class T, class... A> static SP<T> crear(A&&... a) { return SP<T>(new T{std::forward<A>(a)...}); }
    template <class T> static SP<T> load(ASP<T>& c) { return c.load(); }
    template <class T> static void store(ASP<T>& c, SP<T> p) { c.store(std::move(p)); }
    template <class T> static bool cas(ASP<T>& c, const SP<T>& esperado, const SP<T>& nuevo) { SP<T> e = esperado; return c.compare_exchange_strong(e, nuevo); }
};
struct Williams {
    static constexpr const char* nombre = "Williams";
    template <class T> using SP = jss::shared_ptr<T>;
    template <class T> using ASP = jss::atomic_shared_ptr<T>;
    template <class T, class... A> static SP<T> crear(A&&... a) { return SP<T>(new T{std::forward<A>(a)...}); }
    template <class T> static SP<T> load(ASP<T>& c) { return c.load(); }
    template <class T> static void store(ASP<T>& c, SP<T> p) { c.store(std::move(p)); }
    template <class T> static bool cas(ASP<T>& c, const SP<T>& esperado, const SP<T>& nuevo) { SP<T> e = esperado; return c.compare_exchange_strong(e, nuevo); }
};
template <class T> struct CeldaMutex { std::mutex m; std::shared_ptr<T> p; };
struct Mutex {
    static constexpr const char* nombre = "Mutex";
    template <class T> using SP = std::shared_ptr<T>;
    template <class T> using ASP = CeldaMutex<T>;
    template <class T, class... A> static SP<T> crear(A&&... a) { return SP<T>(new T{std::forward<A>(a)...}); }
    template <class T> static SP<T> load(ASP<T>& c) { std::lock_guard<std::mutex> g(c.m); return c.p; }
    template <class T> static void store(ASP<T>& c, SP<T> p) { { std::lock_guard<std::mutex> g(c.m); c.p.swap(p); } }   // el viejo se suelta fuera del candado
    template <class T> static bool cas(ASP<T>& c, const SP<T>& esperado, const SP<T>& nuevo) {
        SP<T> viejo;
        { std::lock_guard<std::mutex> g(c.m); if (c.p != esperado) return false; viejo = std::exchange(c.p, nuevo); }
        return true;
    }
};

/* ---------- pila de Treiber sobre atomic shared pointers ---------- */
template <class I> struct Nodo { long valor; typename I::template SP<Nodo<I>> sig; };
template <class I> struct Pila {
    typename I::template ASP<Nodo<I>> cima;
    void push(long v) {
        auto n = I::template crear<Nodo<I>>(v, I::load(cima));
        while (!I::cas(cima, n->sig, n)) n->sig = I::load(cima);
    }
    bool pop(long& v) {
        for (;;) {
            auto t = I::load(cima);
            if (!t) return false;
            if (I::cas(cima, t, t->sig)) { v = t->valor; return true; }
        }
    }
    long leer() { auto t = I::load(cima); return t ? t->valor : 0; }
};

template <class I> double rendimiento(int hilos, long n, int u, double segundos) {
    std::unique_ptr<Pila<I>[]> pilas(new Pila<I>[n]);
    const int ini = n <= 10 ? 16 : 2;
    for (long k = 0; k < n; k++) for (int j = 0; j < ini; j++) pilas[k].push(j);
    std::atomic<bool> salida{false}; std::atomic<int> listos{0};
    struct alignas(64) Cuenta { long ops = 0; long sumidero = 0; };
    std::vector<Cuenta> cuentas(hilos); std::vector<std::thread> ts;
    for (int t = 0; t < hilos; t++) ts.emplace_back([&, t] {
        std::mt19937_64 rng(1234 + t); Cuenta c; listos++;
        while (listos.load() < hilos) {}
        while (!salida.load(std::memory_order_relaxed)) {
            for (int b = 0; b < 32; b++) {
                long k = rng() % n;
                if ((long)(rng() % 100) < u) { long v; if (pilas[k].pop(v)) pilas[rng() % n].push(v); else pilas[rng() % n].push(1); }
                else c.sumidero += pilas[k].leer();
                c.ops++;
            }
        }
        cuentas[t] = c;
    });
    while (listos.load() < hilos) std::this_thread::yield();
    auto t0 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::duration<double>(segundos));
    salida = true;
    for (auto& th : ts) th.join();
    double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    long total = 0; for (auto& c : cuentas) total += c.ops;
    return total / s / 1e6;   // millones de operaciones por segundo
}

/* ---------- latencia ---------- */
static inline uint64_t tsc() { _mm_lfence(); uint64_t t = __rdtsc(); _mm_lfence(); return t; }
static double ns_por_tick() {
    auto c0 = std::chrono::steady_clock::now(); uint64_t t0 = tsc();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    auto c1 = std::chrono::steady_clock::now(); uint64_t t1 = tsc();
    return std::chrono::duration<double, std::nano>(c1 - c0).count() / double(t1 - t0);
}
struct Dato { long v; };
/* con p = 1 la operación dura menos que el propio reloj: cada muestra es la media de B = 16 operaciones seguidas;
   con p > 1 cada muestra es una sola operación y se descuenta solo el coste mínimo del reloj (así no salen ceros) */
template <class I> std::vector<double> latencia(int p, int op, int muestras, double nspt, double sc_med, double sc_min) {
    using SPD = typename I::template SP<Dato>;
    const int B = p == 1 ? 16 : 1;
    typename I::template ASP<Dato> celda;
    std::vector<SPD> reserva; for (int k = 0; k < 64; k++) reserva.push_back(I::template crear<Dato>(long(k)));
    I::store(celda, reserva[0]);
    std::atomic<bool> salida{false}; std::vector<std::thread> ts;
    for (int t = 1; t < p; t++) ts.emplace_back([&, t] {
        std::mt19937 rng(t);
        while (!salida.load(std::memory_order_relaxed)) {
            if (op == 2) I::store(celda, I::template crear<Dato>(1L));   // todos liberan: cada store suelta el objeto que dejó el anterior
            else I::store(celda, reserva[rng() % 64]);                 // objetos que siguen vivos en la reserva: nadie libera
        }
    });
    std::vector<double> lat; lat.reserve(muestras); long sumidero = 0; SPD lote[16];
    for (int i = -2000; i < muestras; i++) {
        uint64_t a = 0, b = 0;
        if (op == 0) { a = tsc(); for (int k = 0; k < B; k++) lote[k] = I::load(celda); b = tsc(); for (int k = 0; k < B; k++) { if (lote[k]) sumidero += lote[k]->v; lote[k] = SPD(); } }
        else {
            for (int k = 0; k < B; k++) lote[k] = op == 1 ? reserva[(i + k) & 63] : I::template crear<Dato>(2L);
            a = tsc(); for (int k = 0; k < B; k++) I::store(celda, std::move(lote[k])); b = tsc();
        }
        double ns = (b - a) * nspt;
        if (i >= 0) lat.push_back(std::max(0.0, B > 1 ? (ns - sc_med) / B : ns - sc_min));
    }
    salida = true; for (auto& th : ts) th.join();
    if (sumidero == 42) std::puts("");
    return lat;
}
static double percentil(std::vector<double>& v, double q) { size_t k = std::min(v.size() - 1, size_t(q * (v.size() - 1))); std::nth_element(v.begin(), v.begin() + k, v.end()); return v[k]; }

template <class I> void fila_rendimiento(long n, int u, const std::vector<int>& hilos, double seg) {
    for (int h : hilos) { double r = rendimiento<I>(h, n, u, seg); std::printf("%s,%ld,%d,%d,%.3f\n", I::nombre, n, u, h, r); std::fflush(stdout); std::fprintf(stderr, "  %-9s n=%-6ld u=%-2d hilos=%-2d %8.2f Mop/s\n", I::nombre, n, u, h, r); }
}
template <class I> void fila_latencia(int p, int muestras, double nspt, double sc, double scmin) {
    const char* OPS[] = {"load", "store", "store&reclaim"};
    for (int op = 0; op < 3; op++) {
        auto v = latencia<I>(p, op, muestras, nspt, sc, scmin);
        double q[5] = {percentil(v, 0.01), percentil(v, 0.25), percentil(v, 0.50), percentil(v, 0.75), percentil(v, 0.99)};
        std::printf("%s,%s,%d,%.1f,%.1f,%.1f,%.1f,%.1f\n", I::nombre, OPS[op], p, q[0], q[1], q[2], q[3], q[4]); std::fflush(stdout);
        std::fprintf(stderr, "  %-9s %-13s p=%-2d  p1=%8.1f  p25=%8.1f  mediana=%8.1f  p75=%8.1f  p99=%9.1f ns\n", I::nombre, OPS[op], p, q[0], q[1], q[2], q[3], q[4]);
    }
}

int main(int argc, char** argv) {
    std::string modo = argc > 1 ? argv[1] : "rendimiento";
    unsigned hw = std::thread::hardware_concurrency();
    if (modo == "rendimiento") {
        double seg = argc > 2 ? std::atof(argv[2]) : 1.0;
        std::vector<int> hilos = {1, 2, 4, 8, 12, 18};
        std::fprintf(stderr, "rendimiento: %u hilos por hardware, %.1f s por punto\n", hw, seg);
        std::puts("impl,n,u,hilos,mops");
        struct { long n; int u; } paneles[] = {{1, 1}, {1, 10}, {1, 50}, {10, 1}, {10, 10}, {100000, 50}};
        for (auto& pa : paneles) { fila_rendimiento<Autores>(pa.n, pa.u, hilos, seg); fila_rendimiento<Libstdcpp>(pa.n, pa.u, hilos, seg); fila_rendimiento<Williams>(pa.n, pa.u, hilos, seg); fila_rendimiento<Mutex>(pa.n, pa.u, hilos, seg); }
    } else {
        int muestras = argc > 2 ? std::atoi(argv[2]) : 200000;
        double nspt = ns_por_tick();
        std::vector<double> vacio; for (int i = 0; i < 20000; i++) { uint64_t a = tsc(), b = tsc(); vacio.push_back((b - a) * nspt); }
        double sc = percentil(vacio, 0.5); double scmin = *std::min_element(vacio.begin(), vacio.end());
        std::fprintf(stderr, "latencia: %u hilos por hardware, %.3f ns por tick, coste del reloj: mediana %.1f ns, minimo %.1f ns (descontados), %d muestras\n", hw, nspt, sc, scmin, muestras);
        std::fprintf(stderr, "  p = 1: cada muestra es la media de 16 operaciones seguidas; p > 1: cada muestra es una operacion\n");
        std::puts("impl,op,p,p1,p25,p50,p75,p99");
        for (int p : {1, (int)hw}) { fila_latencia<Autores>(p, muestras, nspt, sc, scmin); fila_latencia<Libstdcpp>(p, muestras, nspt, sc, scmin); fila_latencia<Williams>(p, muestras, nspt, sc, scmin); fila_latencia<Mutex>(p, muestras, nspt, sc, scmin); }
    }
}
