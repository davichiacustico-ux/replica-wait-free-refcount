// Parte 3 · std::shared_ptr: el conteo de referencias de C++
// Lo mínimo para el paper (secciones 2.1 y 5.2): un shared_ptr es el "pase" de la analogía.
// Copiarlo suma 1 al contador; destruirlo resta 1; cuando el contador llega a 0, el objeto se
// destruye EN ESE MOMENTO. El contador vive en un bloque de control aparte.
#include <cstdio>
#include <memory>    // shared_ptr, make_shared, weak_ptr

struct Libro {
    int dato;
    explicit Libro(int d) : dato(d) { std::printf("  [se crea el libro %d]\n", dato); }
    ~Libro() { std::printf("  [se libera el libro %d]\n", dato); }   // destructor: se ejecuta al liberar
};

int main() {
    // 1. make_shared crea el objeto y su bloque de control en una sola reserva: rc = 1
    std::shared_ptr<Libro> a = std::make_shared<Libro>(42);
    std::printf("use_count = %ld\n", a.use_count());

    {
        std::shared_ptr<Libro> b = a;          // copiar: rc = 2 (el contador se actualiza con instrucciones atómicas)
        std::printf("tras copiar: use_count = %ld\n", a.use_count());
    }                                          // b se destruye al salir del bloque: rc = 1
    std::printf("tras el bloque: use_count = %ld\n", a.use_count());

    // 2. weak_ptr: apunta sin contar (sirve para romper ciclos). lock() da un shared_ptr si sigue vivo
    std::weak_ptr<Libro> w = a;
    if (auto p = w.lock()) std::printf("el weak_ptr ve el libro %d\n", p->dato);

    // 3. Aliasing: comparte el contador de a, pero apunta a un campo (sección 5.2 del paper)
    std::shared_ptr<int> solo_dato(a, &a->dato);
    std::printf("alias: *solo_dato = %d, use_count = %ld (el alias mantiene vivo el libro)\n", *solo_dato, a.use_count());
    solo_dato.reset();

    // 4. Al soltar la última referencia, el libro se libera ya, sin esperar a nada
    std::printf("a = nullptr...\n");
    a = nullptr;                               // rc = 0: aquí aparece "[se libera el libro 42]"
    std::printf("el weak_ptr ahora ve: %s\n", w.expired() ? "nada (expirado)" : "el libro");

    // Ojo: cada hilo puede copiar SU PROPIO shared_ptr sin problema. Lo peligroso es que varios hilos
    // lean y escriban LA MISMA variable shared_ptr: eso es el atomic shared pointer (parte 6).
    return 0;
}
