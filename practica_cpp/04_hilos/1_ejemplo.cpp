// Parte 4 · Hilos y carreras
// Lo mínimo para el paper: varios hilos comparten la memoria y el sistema puede intercalar sus pasos
// de cualquier forma. x++ son tres pasos (leer, sumar, escribir): si dos hilos se intercalan, se
// pierden sumas. Un candado (std::mutex) lo arregla, pero los demás hilos esperan: es "bloqueante",
// lo que hace la implementación de GCC (Libstdc++) de los experimentos del paper.
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

int main() {
    // 1. Crear hilos: std::thread recibe una función (aquí, una lambda) y join() espera a que acabe
    std::vector<std::thread> hilos;
    for (int id = 0; id < 3; id++)
        hilos.emplace_back([id] { std::printf("hola desde el hilo %d\n", id); });   // [id]: copia id en la lambda
    for (auto& h : hilos) h.join();                                                // el orden de los saludos cambia

    // 2. Carrera de datos: dos hilos suman 1 000 000 cada uno a la misma variable
    //    (volatile solo evita que el compilador junte las sumas; no protege nada)
    volatile int x = 0;
    auto sumar = [&x] { for (int i = 0; i < 1000000; i++) x = x + 1; };   // [&x]: la lambda usa x por referencia
    std::thread a(sumar), b(sumar);
    a.join(); b.join();
    std::printf("sin proteccion: x = %d (deberia ser 2000000)\n", (int)x);

    // 3. Con un candado: correcto, pero cada suma espera a que el otro hilo suelte el candado
    int y = 0;
    std::mutex m;
    auto sumar_con_candado = [&] { for (int i = 0; i < 1000000; i++) { std::lock_guard<std::mutex> g(m); y++; } };
    std::thread c(sumar_con_candado), d(sumar_con_candado);
    c.join(); d.join();
    std::printf("con mutex:      y = %d\n", y);

    // Idea para el paper: la "ventana peligrosa" es el hueco entre leer y escribir. En la carrera
    // read-reclaim, el hueco está entre leer el puntero y sumar 1 a su contador.
    return 0;
}
