// Parte 8 · Solución
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <random>
#include <thread>
#include <vector>

constexpr uint64_t P = 8;
constexpr uint64_t C = 16;
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
    std::atomic<uint64_t> value{0};

    // E3
    void increase_weight(Objeto* p, uint64_t w) {
        incrementar(p, C - w);                                         // reservar ANTES del CAS
        uint64_t actual = w, intentos = 0;
        for (;;) {
            ++intentos;
            uint64_t visto = pack(p, actual);
            if (value.compare_exchange_strong(visto, pack(p, actual + C - w))) { anotar(intentos); return; }
            if (puntero(visto) != p || peso(visto) > actual) break;   // salida temprana (R-2)
            actual = peso(visto);
        }
        anotar(intentos);
        decrementar(p, C - w);                                         // la reserva no llegó al cartel
    }
public:
    // E1
    void store(Objeto* nuevo) {
        incrementar(nuevo, C);
        uint64_t viejo = value.exchange(pack(nuevo, C));
        decrementar(puntero(viejo), peso(viejo));
    }
    // E2
    Objeto* load() {
        uint64_t v = value.fetch_sub(UN_TICKET);
        Objeto* p = puntero(v);
        uint64_t w = peso(v);
        if (p && w - 1 < P) increase_weight(p, w - 1);
        return p;
    }
    uint64_t peso_actual() const { return peso(value.load()); }
};

// E4. En cuanto el objeto aparece en el cartel, otros hilos pueden arrancarle tickets y soltarlos. Si su rc
//     no tuviera ya los C tickets, podría llegar a 0 y liberarse con el cartel apuntándole.
// E5. Con C = P, una recarga ya no deja el peso por encima de P (R-1 garantiza R-2). El peso llega a 0, da
//     la vuelta y aparecen miles de errores y objetos sin liberar: R-1 no es un tecnicismo.
// E6. Sin la salida temprana no hay errores (la reserva respalda lo que se añade), pero el máximo de
//     intentos supera P (en este equipo, entre 41 y 55 con P = 8, según la ejecución): se pierde la cota
//     y deja de ser wait-free.

static int fallos = 0;
static void revisar(bool ok, const char* que) { std::printf("%s %s\n", ok ? "[OK]   " : "[FALTA]", que); if (!ok) fallos++; }

int main() {
    {
        AtomicSharedPtr c; Objeto* o = crear();
        c.store(o);
        revisar(o->rc == 1 + C && c.peso_actual() == C, "E1: store suma C al rc y deja el cartel con peso C");
        Objeto* p = c.load();
        revisar(p == o && o->rc == 1 + C && c.peso_actual() == C - 1, "E2: load devuelve el objeto, baja el peso y NO toca rc");
        std::vector<Objeto*> pases{p};
        for (int i = 0; i < 40; i++) pases.push_back(c.load());
        bool exacto = o->rc == 1 + c.peso_actual() + pases.size() && c.peso_actual() >= P;
        revisar(exacto, "E3: tras 41 loads, rc = 1 + peso + pases y el peso nunca baja de P");
        c.store(nullptr);
        for (Objeto* x : pases) decrementar(x, 1);
        decrementar(o, 1);
        revisar(exacto && !o->vivo && errores == 0, "E1-E3: al soltar todo, el objeto se libera una sola vez");
    }
    errores = 0; vivos = 0; max_intentos = 0;
    AtomicSharedPtr celda; Objeto* primero = crear(); celda.store(primero); decrementar(primero, 1);
    std::vector<std::thread> hs;
    for (int t = 0; t < (int)P; t++)
        hs.emplace_back([&celda, t] {
            std::mt19937 rng(t + 1);
            for (int i = 0; i < 200000; i++) {
                if (rng() % 10 == 0) { Objeto* o = crear(); celda.store(o); decrementar(o, 1); }
                else { Objeto* p = celda.load(); if (p) { comprobar(p); decrementar(p, 1); } }
            }
        });
    for (auto& h : hs) h.join();
    celda.store(nullptr);
    std::printf("   (errores %ld, sin liberar %ld, max. intentos %llu)\n", errores.load(), vivos.load(), (unsigned long long)max_intentos.load());
    revisar(errores == 0 && vivos == 0 && max_intentos >= 1 && max_intentos <= P, "Con 8 hilos: 0 errores, 0 fugas y como mucho P intentos de CAS");
    std::printf(fallos ? "\nTe faltan %d.\n" : "\nTodo correcto.\n", fallos);
    return 0;
}
