// Parte 3 · Solución
#include <cstdio>
#include <memory>
#include <vector>

struct Libro { int dato; };

// E1
long usuarios_tras_copias() {
    auto a = std::make_shared<Libro>(Libro{1});
    std::vector<std::shared_ptr<Libro>> copias(3, a);   // tres copias: +3
    return a.use_count();                                // 1 + 3 = 4
}

// E2
bool sigue_vivo(const std::weak_ptr<Libro>& w) {
    return w.lock() != nullptr;                          // equivalente: !w.expired()
}

// E3
std::shared_ptr<int> alias_del_dato(const std::shared_ptr<Libro>& l) {
    return std::shared_ptr<int>(l, &l->dato);            // comparte el bloque de control de l
}

// E4. Copiar cada uno su propio shared_ptr: no hay carrera, el contador se actualiza de forma
//     atómica. Leer la MISMA variable mientras otro la reasigna: sí hay carrera. Leer esa variable
//     son dos pasos (leer el puntero y sumar 1 al contador, que está en otro sitio) y entre ellos
//     el escritor puede bajar el contador a 0 y liberar el libro. Es la carrera read-reclaim del paper;
//     para eso existe std::atomic<std::shared_ptr<T>> (parte 6).

static int fallos = 0;
static void revisar(bool ok, const char* que) { std::printf("%s %s\n", ok ? "[OK]   " : "[FALTA]", que); if (!ok) fallos++; }

int main() {
    revisar(usuarios_tras_copias() == 4, "E1: original + 3 copias = use_count 4");

    auto l = std::make_shared<Libro>(Libro{7});
    std::weak_ptr<Libro> w = l;
    bool antes = sigue_vivo(w);
    l.reset();
    revisar(antes && !sigue_vivo(w), "E2: vivo antes de soltar el ultimo shared_ptr, muerto despues");

    auto m = std::make_shared<Libro>(Libro{9});
    auto d = alias_del_dato(m);
    revisar(d && *d == 9 && m.use_count() == 2, "E3: el alias ve 9 y el libro tiene use_count 2");
    std::printf(fallos ? "\nTe faltan %d.\n" : "\nTodo correcto.\n", fallos);
    return 0;
}
