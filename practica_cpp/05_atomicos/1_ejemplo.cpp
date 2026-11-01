// Parte 5 · Instrucciones atómicas: fetch_add, exchange y compare-and-swap (CAS)
// Lo mínimo para el paper: son las TRES instrucciones que usa el algoritmo, y todas trabajan
// sobre UNA palabra.
//   fetch_add / fetch_sub -> load del paper: arranca un ticket y devuelve el valor ANTERIOR
//   exchange (swap)       -> store del paper: pone el cartel nuevo y devuelve el viejo
//   compare_exchange      -> recarga (increase_weight): "cámbialo solo si sigue como lo vi"
#include <atomic>
#include <cstdio>
#include <thread>

int main() {
    // 1. fetch_add: sumar de golpe; devuelve lo que había ANTES
    std::atomic<int> x{5};
    int antes = x.fetch_add(1);
    std::printf("fetch_add(1): antes = %d, ahora = %d\n", antes, x.load());

    // 2. Con fetch_add la carrera de la parte 4 desaparece: cada suma es una sola instrucción
    std::atomic<int> c{0};
    auto sumar = [&c] { for (int i = 0; i < 1000000; i++) c.fetch_add(1); };
    std::thread a(sumar), b(sumar);
    a.join(); b.join();
    std::printf("con fetch_add: c = %d (exacto)  -> wait-free: una instruccion pase lo que pase\n", c.load());

    // 3. exchange (swap): pone un valor nuevo y devuelve el viejo, de golpe
    std::atomic<int> cartel{1};
    int viejo = cartel.exchange(2);
    std::printf("exchange(2): devolvio %d, ahora vale %d\n", viejo, cartel.load());

    // 4. compare_exchange_strong (CAS): si vale 'esperado', pone 'nuevo' y devuelve true.
    //    Si no, devuelve false y ESCRIBE EN 'esperado' lo que vio (el cas.original del paper).
    int esperado = 2;
    bool ok = cartel.compare_exchange_strong(esperado, 3);
    std::printf("CAS(2 -> 3): %s, cartel = %d\n", ok ? "exito" : "fallo", cartel.load());
    esperado = 2;                                       // ahora el cartel vale 3, no 2
    ok = cartel.compare_exchange_strong(esperado, 9);
    std::printf("CAS(2 -> 9): %s, y 'esperado' quedo en %d (lo que habia)\n", ok ? "exito" : "fallo", esperado);

    // 5. El bucle CAS: hacer cualquier cambio atómico reintentando si otro se adelanta.
    //    Aquí: duplicar el valor. Es lock-free: si mi CAS falla es porque otro tuvo éxito.
    std::atomic<int> v{10};
    int visto = v.load();
    while (!v.compare_exchange_strong(visto, visto * 2)) { /* 'visto' ya trae el valor nuevo: reintentar */ }
    std::printf("bucle CAS: 10 * 2 = %d\n", v.load());

    // 6. ¿De verdad son una instrucción del procesador? is_lock_free() lo confirma para 64 bits
    std::atomic<unsigned long long> palabra{0};
    std::printf("atomic de 64 bits lock-free: %s\n", palabra.is_lock_free() ? "si" : "no");
    return 0;
}
