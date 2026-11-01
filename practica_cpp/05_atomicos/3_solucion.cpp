// Parte 5 · Solución
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

// E1
long contar_atomico(int n_hilos, int por_hilo) {
    std::atomic<long> total{0};
    std::vector<std::thread> hs;
    for (int t = 0; t < n_hilos; t++)
        hs.emplace_back([&total, por_hilo] { for (int i = 0; i < por_hilo; i++) total.fetch_add(1); });
    for (auto& h : hs) h.join();
    return total.load();
}

// E2
int cambiar(std::atomic<int>& a, int v) {
    return a.exchange(v);                    // swap: una instrucción, devuelve el valor anterior
}

// E3
void maximo_atomico(std::atomic<int>& a, int v) {
    int visto = a.load();
    while (visto < v && !a.compare_exchange_strong(visto, v)) {
        // si falla, 'visto' trae el valor actual: se vuelve a comprobar si v sigue siendo mayor
    }
}

// E4
bool cas_con_visto(std::atomic<int>& a, int esperado, int nuevo, int& visto) {
    visto = esperado;
    bool ok = a.compare_exchange_strong(visto, nuevo);   // si falla, escribe en 'visto' lo que había
    if (ok) visto = esperado;                            // si tuvo éxito, lo que había era el esperado
    return ok;
}

static int fallos = 0;
static void revisar(bool ok, const char* que) { std::printf("%s %s\n", ok ? "[OK]   " : "[FALTA]", que); if (!ok) fallos++; }

int main() {
    revisar(contar_atomico(4, 250000) == 1000000, "E1: 4 hilos x 250000 = 1000000 sin candado");

    std::atomic<int> c{7};
    int viejo = cambiar(c, 8);
    revisar(viejo == 7 && c.load() == 8, "E2: cambiar(c, 8) devuelve 7 y deja 8");

    std::atomic<int> m{0};
    std::vector<std::thread> hs;
    for (int t = 0; t < 4; t++) hs.emplace_back([&m, t] { for (int i = 0; i < 1000; i++) maximo_atomico(m, t * 1000 + i); });
    for (auto& h : hs) h.join();
    revisar(m.load() == 3999, "E3: con 4 hilos, el maximo final es 3999");

    std::atomic<int> a{5}; int visto = -1;
    bool ok1 = cas_con_visto(a, 5, 6, visto);
    bool ok2 = cas_con_visto(a, 5, 9, visto);
    revisar(ok1 && !ok2 && visto == 6 && a.load() == 6, "E4: el primer CAS cambia 5->6; el segundo falla y ve 6");
    std::printf(fallos ? "\nTe faltan %d.\n" : "\nTodo correcto.\n", fallos);
    return 0;
}
