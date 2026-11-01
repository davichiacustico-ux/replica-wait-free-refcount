// Parte 7 · Ejercicios mínimos: escribe tú la versión ingenua y provoca la carrera
// Completa los TODO y ejecuta: cada comprobación debe decir [OK].
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
    // E1. store: la celda pasa a apuntar a 'nuevo'. Orden correcto: primero incrementar 'nuevo'
    //     (la celda será una referencia), después exchange, después decrementar el viejo.
    void store(Objeto* nuevo) {
        // TODO
    }
    // E2. load, versión ingenua: leer el puntero, std::this_thread::yield(), y después incrementar.
    Objeto* load() {
        // TODO
        return nullptr;
    }
};

// E3. (para pensar) Si cambias el orden de load (incrementar antes de leer), ¿se arregla? ¿Por qué no?
// E4. (para pensar) ¿Qué instrucción haría falta para leer el puntero y sumar 1 al rc a la vez?
//     ¿Existe en los procesadores actuales?

static int fallos = 0;
static void revisar(bool ok, const char* que) { std::printf("%s %s\n", ok ? "[OK]   " : "[FALTA]", que); if (!ok) fallos++; }

int main() {
    // Con un solo hilo, la versión ingenua es correcta
    {
        Ingenuo c; Objeto* o = crear();
        c.store(o); decrementar(o);                         // solo queda la referencia de la celda
        bool r1 = o->rc == 1;
        revisar(r1, "E1: tras store y soltar la propia, rc = 1 (la de la celda)");
        Objeto* p = c.load();
        revisar(p == o && o->rc == 2, "E2: load devuelve el objeto y suma 1 a su rc");
        decrementar(p); c.store(nullptr);
        revisar(r1 && !o->vivo && errores == 0, "E1: al vaciar la celda el objeto se libera, sin errores");
    }
    // Con 8 hilos aparece la carrera
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
