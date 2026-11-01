// Parte 8 · El algoritmo del paper en miniatura (algoritmos 1 y 2)
// Junta todo lo anterior: la celda es UNA palabra atómica de 64 bits con (peso | dirección).
//   store: rc(nuevo) += C;  viejo = exchange(celda, pack(nuevo, C));  rc(viejo) -= peso(viejo)
//   load : v = fetch_sub(celda, 2^48)   <- leer el puntero Y arrancar un ticket en UNA instrucción
//          si el peso que queda < P: increase_weight (recargar)
//   increase_weight: reservar C - w en rc, bucle CAS, salir si otro libro o si el peso SUBIÓ (R-2)
// C y P son minúsculos (P = 8, C = 2P) para que las recargas ocurran constantemente.
// Experimentos: compila con -DSIN_SALIDA (quita la salida temprana) o con -DVAR_C=8 (C = P, rompe R-1).
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <random>
#include <thread>
#include <vector>

#ifndef VAR_C
#define VAR_C 16
#endif
constexpr uint64_t P = 8;                       // máximo de hilos
constexpr uint64_t C = VAR_C;                   // R-1: C >= 2P
constexpr uint64_t UN_TICKET = uint64_t(1) << 48;

struct Objeto { std::atomic<uint64_t> rc{1}; std::atomic<bool> vivo{true}; };
std::atomic<long> errores{0}, vivos{0};
std::atomic<uint64_t> max_intentos{0};
Objeto* crear()                        { vivos++; return new Objeto; }
void comprobar(Objeto* o)              { if (!o->vivo) errores++; }
void incrementar(Objeto* o, uint64_t n) { if (!o) return; comprobar(o); o->rc.fetch_add(n); }
void decrementar(Objeto* o, uint64_t n) { if (!o) return; comprobar(o); if (o->rc.fetch_sub(n) == n) { o->vivo = false; vivos--; } }

uint64_t pack(Objeto* p, uint64_t w) { return (w << 48) | reinterpret_cast<uintptr_t>(p); }
Objeto*  puntero(uint64_t v)         { return reinterpret_cast<Objeto*>(v & (UN_TICKET - 1)); }
uint64_t peso(uint64_t v)            { return v >> 48; }
void anotar(uint64_t n) { uint64_t m = max_intentos; while (n > m && !max_intentos.compare_exchange_weak(m, n)) {} }

class AtomicSharedPtr {
    std::atomic<uint64_t> value{0};                          // el cartel: (null, peso 0)

    void increase_weight(Objeto* p, uint64_t w) {           // Algoritmo 2 (w < P)
        incrementar(p, C - w);                              // (1) reservar en rc ANTES del CAS
        uint64_t actual = w, intentos = 0;
        for (;;) {
            ++intentos;
            uint64_t visto = pack(p, actual);
            if (value.compare_exchange_strong(visto, pack(p, actual + C - w))) { anotar(intentos); return; }  // (2) recargué yo
#ifdef SIN_SALIDA
            if (puntero(visto) != p) break;                  // variante rota: reintenta aunque el peso haya subido
#else
            if (puntero(visto) != p || peso(visto) > actual) break;   // (3) otro libro u otro ya recargó: salir
#endif
            actual = peso(visto);                            // (4) alguien arrancó otro ticket: reintentar
        }
        anotar(intentos);
        decrementar(p, C - w);                               // devolver la reserva
    }
public:
    void store(Objeto* nuevo) {                              // Algoritmo 1
        incrementar(nuevo, C);                               // respaldar ANTES de publicar
        uint64_t viejo = value.exchange(pack(nuevo, C));     // swap: el cartel cambia de golpe
        decrementar(puntero(viejo), peso(viejo));            // restar solo los tickets sobrantes
    }
    Objeto* load() {                                         // Algoritmo 1
        uint64_t v = value.fetch_sub(UN_TICKET);             // UNA instrucción: arranca un ticket y lee el puntero
        Objeto* p = puntero(v);
        uint64_t w = peso(v);                                // peso ANTES de arrancar
        if (p && w == 0) errores++;                          // habría quedado negativo: no debe pasar nunca
        if (p && w - 1 < P) increase_weight(p, w - 1);
        return p;                                            // una referencia de peso 1, ya pagada
    }
};

int main() {
    AtomicSharedPtr celda;
    Objeto* primero = crear(); celda.store(primero); decrementar(primero, 1);
    std::vector<std::thread> hs;
    for (int t = 0; t < (int)P; t++)
        hs.emplace_back([&celda, t] {
            std::mt19937 rng(t + 1);
            for (int i = 0; i < 300000; i++) {
                if (rng() % 10 == 0) { Objeto* o = crear(); celda.store(o); decrementar(o, 1); }      // 10 %: store
                else { Objeto* p = celda.load(); if (p) { comprobar(p); decrementar(p, 1); } }      // 90 %: load, usar, soltar
            }
        });
    for (auto& h : hs) h.join();
    celda.store(nullptr);                                    // vaciar: todo debe quedar liberado
    std::printf("P = %llu, C = %llu, %d hilos\n", (unsigned long long)P, (unsigned long long)C, (int)P);
    std::printf("accesos a objetos liberados: %ld   (correcto: 0)\n", errores.load());
    std::printf("objetos sin liberar:         %ld   (correcto: 0)\n", vivos.load());
    std::printf("max. intentos de CAS:        %llu   (cota del paper: P = %llu)\n", (unsigned long long)max_intentos.load(), (unsigned long long)P);
    return 0;
}
