// Parte 1 · Punteros, new/delete y use-after-free
// Lo mínimo para el paper: un puntero guarda una DIRECCIÓN, no el objeto. El objeto vive en el
// montón (heap) hasta que alguien lo libera con delete. Usar un puntero a memoria ya liberada es un
// use-after-free: el fallo que el paper evita (sección 2.2).
#include <cstdio>

struct Libro {      // el "objeto" del paper (en la analogía, el libro)
    int rc;         // su contador de referencias (la "hoja de usuarios")
    int dato;
};

int main() {
    // 1. Un puntero es una variable que guarda la dirección de otra
    int x = 5;
    int* p = &x;                 // &x = "la dirección de x"
    *p = 7;                      // *p = "lo que hay en esa dirección": cambia x
    std::printf("x = %d, guardada en la direccion %p\n", x, (void*)p);

    // 2. Objetos en el montón: new reserva, delete libera
    Libro* a = new Libro{1, 42}; // rc = 1: una referencia, la de quien lo crea
    Libro* b = a;                // b apunta al MISMO libro: no es una copia del libro
    b->dato = 99;                // p->campo es lo mismo que (*p).campo
    std::printf("a->dato = %d  (a y b valen %p)\n", a->dato, (void*)a);

    // 3. Contar referencias a mano: cada puntero "oficial" suma 1 al contador
    a->rc += 1;                  // ahora a y b cuentan: rc = 2
    std::printf("rc = %d\n", a->rc);

    // 4. Liberar
    delete a;                    // la memoria vuelve al gestor de memoria
    a = nullptr;                 // buena costumbre: un puntero nulo no apunta a nada
    // b SIGUE guardando la dirección vieja: es un puntero colgante (dangling pointer).
    // b->dato = 1;              // ¡NO lo hagas! Sería un use-after-free: comportamiento indefinido.
    //                           // Puede funcionar, corromper otro objeto o romper el programa más tarde.
    std::printf("b sigue valiendo %p, pero ya no se puede usar\n", (void*)b);
    // Al compilar, g++ avisa aquí (-Wuse-after-free): hasta mirar el valor de un puntero tras delete
    // es sospechoso. Con hilos, el compilador ya no puede avisar: por eso existe el paper.

    // Idea para el paper: el lector de la carrera tiene un puntero como b. Cuando por fin
    // intenta sumar 1 al contador (b->rc += 1), el libro ya se liberó.
    return 0;
}
