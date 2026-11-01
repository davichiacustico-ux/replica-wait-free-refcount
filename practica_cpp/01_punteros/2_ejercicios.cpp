// Parte 1 · Ejercicios mínimos de punteros
// Completa los TODO y ejecuta: cada comprobación debe decir [OK].
#include <cstdio>

struct Libro { int rc; int dato; };

// E1. Escribe v en la variable a la que apunta p.
void poner(int* p, int v) {
    // TODO
}

// E2. Crea un Libro nuevo en el montón con rc = 1 y el dato indicado, y devuelve su puntero.
Libro* crear(int dato) {
    // TODO
    return nullptr;
}

// E3. "Soltar una referencia": resta 1 a l->rc. Si llega a 0, libera el libro con delete
//     y deja el puntero en nullptr (por eso se recibe por referencia: Libro*&).
//     Así funciona soltar un shared_ptr.
void soltar(Libro*& l) {
    // TODO
}

// E4. (para pensar, sin código) En la carrera del paper, ¿por qué el lector no puede
//     "comprobar después" si el libro sigue existiendo antes de sumar 1 a su contador?

static int fallos = 0;
static void revisar(bool ok, const char* que) { std::printf("%s %s\n", ok ? "[OK]   " : "[FALTA]", que); if (!ok) fallos++; }

int main() {
    int y = 0;
    poner(&y, 3);
    revisar(y == 3, "E1: poner(&y, 3) deja y = 3");

    Libro* l = crear(10);
    revisar(l && l->rc == 1 && l->dato == 10, "E2: crear(10) da rc = 1 y dato = 10");

    if (l) {
        l->rc = 2;
        soltar(l);
        revisar(l != nullptr && l->rc == 1, "E3: con rc = 2, soltar deja rc = 1 y no libera");
        if (l) soltar(l);
        revisar(l == nullptr, "E3: con rc = 1, soltar libera y pone el puntero a nullptr");
    } else {
        revisar(false, "E3: (antes hay que resolver E2)");
    }
    std::printf(fallos ? "\nTe faltan %d.\n" : "\nTodo correcto.\n", fallos);
    return 0;
}
