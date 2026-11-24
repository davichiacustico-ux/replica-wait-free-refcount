// ancho_banda.cpp — cuánto mueve la RAM de verdad: el techo contra el que se comparan las pilas con n = 10⁶.
// Cada hilo recorre su trozo de dos vectores de 4 GiB, muy por encima de los 36 MB de la L3, para que nada quede en caché.
//   lectura y copia van en orden, el mejor caso para la RAM. azar lee líneas de 64 bytes sueltas, como las pilas con n = 10⁶.
//   ./ancho_banda <lectura|copia|azar> <hilos> <segundos>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 4) { std::fputs("uso: ancho_banda lectura|copia|azar hilos segundos\n", stderr); return 2; }
    std::string modo = argv[1];
    bool copia = modo == "copia", azar = modo == "azar";
    int hilos = std::atoi(argv[2]);
    double segundos = std::atof(argv[3]);
    size_t total = size_t(4) << 30, trozo = total / hilos / 4096 * 4096;
    const size_t accesos = size_t(1) << 22;
    char* a = static_cast<char*>(std::aligned_alloc(4096, total));
    char* b = static_cast<char*>(std::aligned_alloc(4096, total));
    std::memset(a, 1, total);
    std::memset(b, 2, total);

    struct alignas(64) Cuenta { uint64_t pasadas = 0, sumidero = 0; };
    std::vector<Cuenta> cuentas(hilos);
    std::atomic<bool> parar{false};
    std::vector<std::thread> grupo;
    auto inicio = std::chrono::steady_clock::now();
    for (int h = 0; h < hilos; h++) grupo.emplace_back([&, h] {
        char* origen = a + h * trozo;
        char* destino = b + h * trozo;
        while (!parar.load(std::memory_order_relaxed)) {
            if (copia) std::memcpy(destino, origen, trozo);
            else if (azar) {
                uint64_t x = 88172645463325252ULL + h + cuentas[h].pasadas * 1000003, suma = 0;
                for (size_t i = 0; i < accesos; i++) {
                    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
                    suma += *reinterpret_cast<const uint64_t*>(origen + x % (trozo / 64) * 64);
                }
                cuentas[h].sumidero += suma;
            } else {
                uint64_t suma = 0;
                auto* p = reinterpret_cast<const uint64_t*>(origen);
                for (size_t i = 0; i < trozo / 8; i++) suma += p[i];
                cuentas[h].sumidero += suma;
            }
            cuentas[h].pasadas++;
        }
    });
    std::this_thread::sleep_for(std::chrono::duration<double>(segundos));
    parar = true;
    for (auto& t : grupo) t.join();
    double dur = std::chrono::duration<double>(std::chrono::steady_clock::now() - inicio).count();
    uint64_t pasadas = 0, sumidero = 0;
    for (auto& c : cuentas) { pasadas += c.pasadas; sumidero += c.sumidero; }
    double por_pasada = azar ? accesos * 64.0 : trozo * (copia ? 2.0 : 1.0);
    std::printf("Movido = %.2f GB/s (sumidero %llu)\n", pasadas * por_pasada / dur / 1e9, (unsigned long long)sumidero);
}
