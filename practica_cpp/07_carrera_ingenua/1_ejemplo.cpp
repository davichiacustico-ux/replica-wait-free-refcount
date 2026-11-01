// Parte 7 · La carrera read-reclaim, de verdad
// Lo mínimo para el paper (secciones 1 y 2.2): un atomic shared pointer hecho "a lo fácil".
// El puntero está en la celda compartida y el contador (rc) dentro del objeto: SON DOS SITIOS.
// load tiene que dar dos pasos: (1) leer el puntero y (2) sumar 1 a su rc. Entre los dos, un store
// puede bajar el rc del objeto viejo a 0 y liberarlo: el lector suma 1 a un objeto muerto.
//
// Para ver el fallo sin romper el programa, "liberar" no hace delete: marca el objeto como muerto.
// Así cada acceso posterior se detecta siempre. yield() ensancha la ventana para que el fallo
// aparezca en segundos (en un programa real pasaría una vez cada muchos millones de operaciones).
#include <atomic>
#include <cstdio>
#include <random>
#include <thread>
#include <vector>

struct Objeto {
    std::atomic<long> rc{1};          // nace con 1: la referencia de quien lo crea
    std::atomic<bool> vivo{true};
};
std::atomic<long> errores{0}, vivos{0};

Objeto* crear()                 { vivos++; return new Objeto; }
void comprobar(Objeto* o)       { if (!o->vivo) errores++; }                 // ¿toco un objeto liberado?
void incrementar(Objeto* o)     { if (!o) return; comprobar(o); o->rc.fetch_add(1); }
void decrementar(Objeto* o)     { if (!o) return; comprobar(o); if (o->rc.fetch_sub(1) == 1) { o->vivo = false; vivos--; } }

class Ingenuo {                       // atomic shared pointer SIN el truco del paper
    std::atomic<Objeto*> ptr{nullptr};
public:
    void store(Objeto* nuevo) {
        incrementar(nuevo);                       // la celda será una referencia más
        Objeto* viejo = ptr.exchange(nuevo);      // cambiar el cartel de golpe
        decrementar(viejo);                       // la celda suelta el viejo: puede llegar a 0
    }
    Objeto* load() {
        Objeto* p = ptr.load();                   // paso 1: leer el puntero
        std::this_thread::yield();                //   <- la ventana peligrosa (aquí, ensanchada)
        incrementar(p);                           // paso 2: apuntarse... ¡p puede estar ya liberado!
        return p;
    }
};

int main() {
    Ingenuo celda;
    Objeto* primero = crear(); celda.store(primero); decrementar(primero);
    std::vector<std::thread> hs;
    for (int t = 0; t < 8; t++)
        hs.emplace_back([&celda, t] {
            std::mt19937 rng(t + 1);
            for (int i = 0; i < 100000; i++) {
                if (rng() % 10 == 0) { Objeto* o = crear(); celda.store(o); decrementar(o); }   // 10 %: escritor
                else { Objeto* p = celda.load(); if (p) { comprobar(p); decrementar(p); } }    // 90 %: lector
            }
        });
    for (auto& h : hs) h.join();
    celda.store(nullptr);
    std::printf("accesos a objetos liberados: %ld\n", errores.load());
    std::printf("objetos sin liberar:         %ld (negativo = dobles liberaciones)\n", vivos.load());
    // Los números cambian en cada ejecución. Lo importante: no son 0. Esa es la carrera del paper.
    return 0;
}
