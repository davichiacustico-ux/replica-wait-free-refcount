// Parte 8 · Ejercicios: escribe tú el algoritmo del paper
// Las piezas de ayuda (pack, puntero, peso, incrementar, decrementar) ya están. Completa los tres
// TODO de la clase y ejecuta: cada comprobación debe decir [OK].
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <random>
#include <thread>
#include <vector>

constexpr uint64_t P = 8;
constexpr uint64_t C = 16;                       // R-1: C >= 2P
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

    // E3. Recarga (algoritmo 2). w = peso que dejó el load (w < P).
    //     1) reserva C - w en el rc de p;  2) bucle: CAS de pack(p, actual) a pack(p, actual + C - w);
    //     si funciona, anotar(intentos) y salir;  3) si falla y el cartel muestra otro puntero o un peso
    //     MAYOR que actual: salir del bucle;  4) si no, actual = peso visto y reintentar.
    //     Al salir sin éxito: anotar(intentos) y devolver la reserva.
    void increase_weight(Objeto* p, uint64_t w) {
        // TODO
    }
public:
    // E1. store (algoritmo 1): sumar C al rc de 'nuevo', exchange con pack(nuevo, C),
    //     restar al objeto viejo su peso sobrante.
    void store(Objeto* nuevo) {
        // TODO
    }
    // E2. load (algoritmo 1): UN fetch_sub de UN_TICKET; separar puntero y peso (el de ANTES de restar);
    //     si p no es nulo y el peso que queda (w - 1) es menor que P, llamar a increase_weight(p, w - 1).
    Objeto* load() {
        // TODO
        return nullptr;
    }
    uint64_t peso_actual() const { return peso(value.load()); }   // ayuda para las comprobaciones
};

// E4. (para pensar) ¿Por qué store suma C al objeto nuevo ANTES del exchange y no después?
// E5. (experimento) Cambia C a 8 (= P, rompe R-1) y vuelve a ejecutar. ¿Qué pasa con los errores?
// E6. (experimento) En E3, quita "|| peso(visto) > actual". ¿Qué pasa con el máximo de intentos?

static int fallos = 0;
static void revisar(bool ok, const char* que) { std::printf("%s %s\n", ok ? "[OK]   " : "[FALTA]", que); if (!ok) fallos++; }

int main() {
    {   // un solo hilo: la contabilidad exacta
        AtomicSharedPtr c; Objeto* o = crear();             // rc = 1 (mi propia referencia)
        c.store(o);
        revisar(o->rc == 1 + C && c.peso_actual() == C, "E1: store suma C al rc y deja el cartel con peso C");
        Objeto* p = c.load();
        revisar(p == o && o->rc == 1 + C && c.peso_actual() == C - 1, "E2: load devuelve el objeto, baja el peso y NO toca rc");
        std::vector<Objeto*> pases{p};
        for (int i = 0; i < 40; i++) pases.push_back(c.load());   // obliga a recargar varias veces
        bool exacto = o->rc == 1 + c.peso_actual() + pases.size() && c.peso_actual() >= P;
        revisar(exacto, "E3: tras 41 loads, rc = 1 + peso + pases y el peso nunca baja de P");
        c.store(nullptr);                                   // el cartel suelta sus tickets sobrantes
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
