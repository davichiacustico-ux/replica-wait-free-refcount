// Boceto didáctico del algoritmo de de Wolff, Anderson, Keller y Seletskiy (Euro-Par 2024),
// tomado de la explicación (Parte 11). NO es el código de los autores.
#pragma once
#include <atomic>
#include <cstdint>

constexpr int      BITS_DIR    = 48;
constexpr uint64_t UN_TICKET   = uint64_t(1) << BITS_DIR;   // = pack(null, 1)
constexpr uint64_t MASCARA_DIR = UN_TICKET - 1;
#ifndef VAR_P
#define VAR_P 8
#endif
#ifndef VAR_C
#define VAR_C (2 * VAR_P)
#endif
constexpr uint64_t P = VAR_P;
constexpr uint64_t C = VAR_C;

struct Objeto {
    std::atomic<uint64_t> rc{1};
    std::atomic<bool> vivo{true};
    int dato = 42;
};
std::atomic<long> objetos_vivos{0};
std::atomic<long> errores{0};
inline Objeto* crear() { objetos_vivos++; return new Objeto; }
inline void comprobar(Objeto* o) { if (!o->vivo.load()) errores++; }
inline void liberar(Objeto* o) { o->vivo.store(false); objetos_vivos--; }
inline void incrementar(Objeto* o, uint64_t n) { if (!o) return; comprobar(o); o->rc.fetch_add(n); }
inline void decrementar(Objeto* o, uint64_t n) { if (!o) return; comprobar(o); if (o->rc.fetch_sub(n) == n) liberar(o); }

inline uint64_t pack(Objeto* p, uint64_t peso) { return (peso << BITS_DIR) | reinterpret_cast<std::uintptr_t>(p); }
inline Objeto*  puntero(uint64_t v) { return reinterpret_cast<Objeto*>(v & MASCARA_DIR); }
inline uint64_t peso(uint64_t v)    { return v >> BITS_DIR; }

std::atomic<uint64_t> max_intentos{0};

class AtomicSharedPtr {
    std::atomic<uint64_t> value{0};
    void increase_weight(Objeto* p, uint64_t w) {            // Algoritmo 2
        incrementar(p, C - w);                                // (1) reservar ANTES del CAS
        uint64_t actual = w, intentos = 0;
        for (;;) {
            ++intentos;
            uint64_t visto = pack(p, actual);
            if (value.compare_exchange_strong(visto, pack(p, actual + C - w))) { anotar(intentos); return; }
#ifdef SIN_SALIDA
            if (puntero(visto) != p) break;
#else
            if (puntero(visto) != p || peso(visto) > actual) break;   // (3) ya no hace falta
#endif
            actual = peso(visto);                                      // (4) reintentar
        }
        anotar(intentos);
        decrementar(p, C - w);
    }
    static void anotar(uint64_t n) { uint64_t m = max_intentos.load(); while (n > m && !max_intentos.compare_exchange_weak(m, n)) {} }
public:
    void store(Objeto* nuevo) {                                // Algoritmo 1
        incrementar(nuevo, C);
        uint64_t viejo = value.exchange(pack(nuevo, C));
        decrementar(puntero(viejo), peso(viejo));
    }
    Objeto* load() {                                           // Algoritmo 1
        uint64_t v = value.fetch_sub(UN_TICKET);
        Objeto* p = puntero(v);
        uint64_t w = peso(v);
        if (p && w == 0) errores++;
        if (p && w - 1 < P) increase_weight(p, w - 1);
        return p;
    }
};
