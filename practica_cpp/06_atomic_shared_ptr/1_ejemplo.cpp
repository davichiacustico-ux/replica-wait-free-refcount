// Parte 6 · std::atomic<std::shared_ptr<T>> (C++20): el "atomic shared pointer" del paper
// Lo mínimo para el paper (secciones 1 y 2.1): una variable shared_ptr que muchos hilos leen (load)
// mientras otro la cambia (store). load te da TU PROPIO shared_ptr: mientras lo tengas, el objeto
// no puede liberarse. El paper propone una forma rápida y wait-free de implementar esta pieza.
// En GCC (libstdc++) está hecha con candados: is_lock_free() devuelve false. Es el rival
// «Libstdc++» de los experimentos, el que se hunde cuando hay mucha contención.
#include <atomic>
#include <cstdio>
#include <memory>
#include <thread>
#include <vector>

struct Config { int version; };

int main() {
    std::atomic<std::shared_ptr<Config>> config;               // el "cartel"
    config.store(std::make_shared<Config>(Config{0}));
    std::printf("is_lock_free = %s (GCC usa candados por dentro)\n", config.is_lock_free() ? "si" : "no");

    // 1. Un escritor publica versiones nuevas; tres lectores leen sin parar
    std::atomic<bool> fin{false};
    std::thread escritor([&] {
        for (int v = 1; v <= 20000; v++) config.store(std::make_shared<Config>(Config{v}));   // store = cambiar el cartel
        fin = true;
    });
    std::vector<std::thread> lectores;
    std::atomic<long> lecturas{0}, retrocesos{0};
    for (int t = 0; t < 3; t++)
        lectores.emplace_back([&] {
            int ultima = -1;
            while (!fin) {
                std::shared_ptr<Config> c = config.load();     // load = llevarse un pase propio
                if (c->version < ultima) retrocesos++;          // nunca debería ver una versión anterior
                ultima = c->version;                            // usar *c es seguro: c mantiene vivo el objeto
                lecturas++;
            }
        });
    escritor.join();
    for (auto& l : lectores) l.join();
    std::printf("%ld lecturas, %ld retrocesos, version final %d\n", lecturas.load(), retrocesos.load(), config.load()->version);

    // 2. exchange (swap): cambia el cartel y te devuelve el pase del objeto viejo
    auto viejo = config.exchange(std::make_shared<Config>(Config{100}));
    std::printf("exchange: el viejo era la version %d; ahora %d\n", viejo->version, config.load()->version);

    // 3. compare_exchange_strong: cambiar solo si el cartel sigue mostrando el objeto esperado
    std::shared_ptr<Config> esperado = config.load();
    bool ok = config.compare_exchange_strong(esperado, std::make_shared<Config>(Config{101}));
    std::printf("CAS con el esperado correcto: %s -> version %d\n", ok ? "exito" : "fallo", config.load()->version);
    return 0;
}
