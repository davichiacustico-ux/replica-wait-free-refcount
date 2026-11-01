// Parte 7 · Solución
#include <atomic>
#include <cstdio>
#include <random>
#include <thread>
#include <vector>

struct Objeto { std::atomic<long> rc{1}; std::atomic<bool> vivo{true}; };
std::atomic<long> errores{0}, vivos{0};
Objeto* crear()             { vivos++; return new Objeto; }
void comprobar(Objeto* o)   { if (!o->vivo) errores++; }
void incrementar(Objeto* o) { if (!o) return; comprobar(o); o->rc.fetch_add(1); }
void decrementar(Objeto* o) { if (!o) return; comprobar(o); if (o->rc.fetch_sub(1) == 1) { o->vivo = false; vivos--; } }

class Ingenuo {
    std::atomic<Objeto*> ptr{nullptr};
public:
    // E1
    void store(Objeto* nuevo) {
        incrementar(nuevo);
        Objeto* viejo = ptr.exchange(nuevo);
        decrementar(viejo);
    }
    // E2
    Objeto* load() {
        Objeto* p = ptr.load();
        std::this_thread::yield();
        incrementar(p);
        return p;
    }
};

// E3. No. Para incrementar el rc hay que saber de QUÉ objeto, es decir, haber leído ya el puntero.
//     Cualquier orden deja dos pasos separados (leer la celda, escribir en el objeto), y entre ellos
//     el escritor puede liberar el objeto.
// E4. Un CAS sobre dos palabras en sitios distintos (DCAS): no existe en el hardware actual. Por eso el
//     paper mete el "contador de la celda" (el peso) en la MISMA palabra que el puntero (partes 2 y 8).

static int fallos = 0;
static void revisar(bool ok, const char* que) { std::printf("%s %s\n", ok ? "[OK]   " : "[FALTA]", que); if (!ok) fallos++; }

int main() {
    {
        Ingenuo c; Objeto* o = crear();
        c.store(o); decrementar(o);
        bool r1 = o->rc == 1;
        revisar(r1, "E1: tras store y soltar la propia, rc = 1 (la de la celda)");
        Objeto* p = c.load();
        revisar(p == o && o->rc == 2, "E2: load devuelve el objeto y suma 1 a su rc");
        decrementar(p); c.store(nullptr);
        revisar(r1 && !o->vivo && errores == 0, "E1: al vaciar la celda el objeto se libera, sin errores");
    }
    errores = 0; vivos = 0;
    Ingenuo celda; Objeto* primero = crear(); celda.store(primero); decrementar(primero);
    std::vector<std::thread> hs;
    for (int t = 0; t < 8; t++)
        hs.emplace_back([&celda, t] {
            std::mt19937 rng(t + 1);
            for (int i = 0; i < 100000; i++) {
                if (rng() % 10 == 0) { Objeto* o = crear(); celda.store(o); decrementar(o); }
                else { Objeto* p = celda.load(); if (p) { comprobar(p); decrementar(p); } }
            }
        });
    for (auto& h : hs) h.join();
    celda.store(nullptr);
    std::printf("   (accesos a objetos liberados: %ld)\n", errores.load());
    revisar(errores > 0, "Con 8 hilos la carrera aparece: hay accesos a objetos liberados");
    std::printf(fallos ? "\nTe faltan %d.\n" : "\nTodo correcto.\n", fallos);
    return 0;
}
