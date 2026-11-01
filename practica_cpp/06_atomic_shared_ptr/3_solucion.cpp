// Parte 6 · Solución (compila con -std=c++20)
#include <atomic>
#include <cstdio>
#include <memory>
#include <thread>
#include <vector>

struct Config { int version; };
using Cartel = std::atomic<std::shared_ptr<Config>>;

// E1
std::shared_ptr<Config> leer(Cartel& c) {
    return c.load();                                         // un pase propio: suma 1 al contador
}

// E2
void publicar(Cartel& c, int version) {
    c.store(std::make_shared<Config>(Config{version}));      // el objeto viejo se libera cuando lo suelta el último
}

// E3
std::shared_ptr<Config> cambiar(Cartel& c, int version) {
    return c.exchange(std::make_shared<Config>(Config{version}));
}

// E4
bool cambiar_si_es(Cartel& c, std::shared_ptr<Config> esperado, std::shared_ptr<Config> nuevo) {
    return c.compare_exchange_strong(esperado, nuevo);       // compara punteros, no versiones
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
    bool b = cambiar_si_es(c, actual, std::make_shared<Config>(Config{8}));
    revisar(a && !b && c.load()->version == 7, "E4: el primer cambio funciona; el segundo no (ya no es el esperado)");

    std::atomic<bool> fin{false}; std::atomic<long> mal{0};
    std::thread w([&] { for (int v = 10; v < 5000; v++) publicar(c, v); fin = true; });
    std::vector<std::thread> ls;
    for (int t = 0; t < 3; t++) ls.emplace_back([&] { int u = -1; do { auto x = leer(c); if (!x || x->version < u) mal++; else u = x->version; } while (!fin); });   // do-while: al menos una lectura
    w.join(); for (auto& l : ls) l.join();
    revisar(mal.load() == 0, "Con hilos: ningun lector ve un objeto nulo ni una version anterior");
    std::printf(fallos ? "\nTe faltan %d.\n" : "\nTodo correcto.\n", fallos);
    return 0;
}
