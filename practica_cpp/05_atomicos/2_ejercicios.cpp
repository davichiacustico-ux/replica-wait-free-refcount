// Parte 5 · Ejercicios mínimos de atómicos
// Completa los TODO y ejecuta: cada comprobación debe decir [OK].
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

// E1. Como la parte 4, pero sin candado: n_hilos suman con fetch_add sobre un std::atomic<long>.
long contar_atomico(int n_hilos, int por_hilo) {
    std::atomic<long> total{0};
    // TODO
    return total.load();
}

// E2. "store del paper": pon v en a y devuelve lo que había, con UNA instrucción.
int cambiar(std::atomic<int>& a, int v) {
    // TODO
    return 0;
}

// E3. Bucle CAS: haz que a valga max(a, v), aunque otros hilos la cambien a la vez.
void maximo_atomico(std::atomic<int>& a, int v) {
    // TODO
}

// E4. CAS "con lo que vio": intenta cambiar a de 'esperado' a 'nuevo'. Devuelve si tuvo éxito y,
//     en 'visto', el valor que había en a justo en ese momento (como cas.original en el algoritmo 2).
bool cas_con_visto(std::atomic<int>& a, int esperado, int nuevo, int& visto) {
    // TODO
    visto = 0;
    return false;
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
