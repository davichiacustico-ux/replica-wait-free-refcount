// Parte 3 · Ejercicios mínimos de shared_ptr
// Completa los TODO y ejecuta: cada comprobación debe decir [OK].
#include <cstdio>
#include <memory>
#include <vector>

struct Libro { int dato; };

// E1. Crea un libro con make_shared, guarda 3 copias del puntero en un vector
//     y devuelve el use_count resultante (contando también el original).
long usuarios_tras_copias() {
    // TODO
    return 0;
}

// E2. Devuelve true si el libro al que apunta w sigue vivo (usa lock() o expired()).
bool sigue_vivo(const std::weak_ptr<Libro>& w) {
    // TODO
    return false;
}

// E3. Devuelve un shared_ptr<int> que apunte al campo dato de l pero comparta su contador
//     (constructor de aliasing: std::shared_ptr<int>(l, &l->dato)).
std::shared_ptr<int> alias_del_dato(const std::shared_ptr<Libro>& l) {
    // TODO
    return nullptr;
}

// E4. (para pensar) Si dos hilos copian cada uno SU PROPIO shared_ptr del mismo libro a la vez,
//     ¿hay carrera? ¿Y si uno lee la variable compartida `std::shared_ptr<Libro> cartel;`
//     mientras otro hace `cartel = otro;`?

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
