// Parte 1 · Solución
#include <cstdio>

struct Libro { int rc; int dato; };

// E1
void poner(int* p, int v) {
    *p = v;                       // escribir en la dirección que guarda p
}

// E2
Libro* crear(int dato) {
    return new Libro{1, dato};    // nace con rc = 1: la referencia de quien lo crea
}

// E3
void soltar(Libro*& l) {
    l->rc -= 1;
    if (l->rc == 0) {             // nadie más lo usa: se libera EN ESE MOMENTO
        delete l;
        l = nullptr;
    }
}

// E4. Para comprobar si el libro existe tendría que leerlo (por ejemplo, mirar l->rc), y si ya se
//     liberó, esa misma lectura es un use-after-free. Con un puntero no se puede saber si la
//     memoria a la que apunta sigue siendo el mismo objeto: puede estar libre o ser otro objeto.

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
    }
    std::printf(fallos ? "\nTe faltan %d.\n" : "\nTodo correcto.\n", fallos);
    return 0;
}
