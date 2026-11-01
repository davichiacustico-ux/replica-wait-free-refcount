// Parte 6 · Ejercicios mínimos de atomic<shared_ptr> (compila con -std=c++20)
// Completa los TODO y ejecuta: cada comprobación debe decir [OK].
#include <atomic>
#include <cstdio>
#include <memory>
#include <thread>
#include <vector>

struct Config { int version; };
using Cartel = std::atomic<std::shared_ptr<Config>>;

// E1. load: devuelve un shared_ptr propio al objeto que muestra el cartel.
std::shared_ptr<Config> leer(Cartel& c) {
    // TODO
    return nullptr;
}

// E2. store: publica una Config nueva con esa versión.
void publicar(Cartel& c, int version) {
    // TODO
}

// E3. swap: publica una Config nueva y devuelve la que había.
std::shared_ptr<Config> cambiar(Cartel& c, int version) {
    // TODO
    return nullptr;
}

// E4. compare-and-set: publica 'nuevo' solo si el cartel sigue mostrando 'esperado'. Devuelve si lo hizo.
//     (El paper implementa esto en la sección 5.1; en su algoritmo solo es lock-free.)
bool cambiar_si_es(Cartel& c, std::shared_ptr<Config> esperado, std::shared_ptr<Config> nuevo) {
    // TODO
    return false;
}

static int fallos = 0;
static void revisar(bool ok, const char* que) { std::printf("%s %s\n", ok ? "[OK]   " : "[FALTA]", que); if (!ok) fallos++; }

int main() {
    Cartel c;
    c.store(std::make_shared<Config>(Config{0}));

    auto r = leer(c);
    revisar(r && r->version == 0 && r.use_count() >= 2, "E1: leer da un pase propio (use_count >= 2)");

    publicar(c, 5);
    auto r2 = c.load();
    revisar(r2 && r2->version == 5 && r && r->version == 0, "E2: tras publicar(5) el cartel muestra 5 y el pase viejo sigue valido");

    auto viejo = cambiar(c, 6);
    revisar(viejo && viejo->version == 5 && c.load()->version == 6, "E3: cambiar(6) devuelve la 5 y deja la 6");

    auto actual = c.load();
    bool a = cambiar_si_es(c, actual, std::make_shared<Config>(Config{7}));
    bool b = cambiar_si_es(c, actual, std::make_shared<Config>(Config{8}));   // 'actual' ya no está en el cartel
    revisar(a && !b && c.load()->version == 7, "E4: el primer cambio funciona; el segundo no (ya no es el esperado)");

    // Prueba con hilos: un escritor y tres lectores usando tus funciones
    std::atomic<bool> fin{false}; std::atomic<long> mal{0};
    std::thread w([&] { for (int v = 10; v < 5000; v++) publicar(c, v); fin = true; });
    std::vector<std::thread> ls;
    for (int t = 0; t < 3; t++) ls.emplace_back([&] { int u = -1; do { auto x = leer(c); if (!x || x->version < u) mal++; else u = x->version; } while (!fin); });   // do-while: al menos una lectura
    w.join(); for (auto& l : ls) l.join();
    revisar(mal.load() == 0, "Con hilos: ningun lector ve un objeto nulo ni una version anterior");
    std::printf(fallos ? "\nTe faltan %d.\n" : "\nTodo correcto.\n", fallos);
    return 0;
}
