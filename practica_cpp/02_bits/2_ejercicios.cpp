// Parte 2 · Ejercicios mínimos de bits
// Completa los TODO y ejecuta: cada comprobación debe decir [OK].
#include <cstdint>
#include <cstdio>

// E1. Junta dirección (48 bits bajos) y peso (16 bits altos) en una palabra.
uint64_t pack(uint64_t dir, uint64_t peso) {
    // TODO
    return 0;
}

// E2. Sepáralos otra vez.
uint64_t peso(uint64_t v) {
    // TODO
    return 0;
}
uint64_t direccion(uint64_t v) {
    // TODO
    return 0;
}

// E3. Un load del paper: resta UN ticket al peso con una sola resta, sin tocar la dirección.
uint64_t arrancar_ticket(uint64_t v) {
    // TODO
    return v;
}

// E4. Con bits de alineación: las direcciones son múltiplos de 8, así que sus 3 últimos bits valen 0.
//     Guarda la dirección sin esos 3 ceros (dir >> 3, 45 bits) y el peso en los 19 bits de arriba.
uint64_t pack19(uint64_t dir, uint64_t peso) {
    // TODO
    return 0;
}
uint64_t direccion19(uint64_t v) {
    // TODO (vuelve a poner los 3 ceros)
    return 0;
}

static int fallos = 0;
static void revisar(bool ok, const char* que) { std::printf("%s %s\n", ok ? "[OK]   " : "[FALTA]", que); if (!ok) fallos++; }

int main() {
    const uint64_t dir = 0x7F3A12345670;
    uint64_t v = pack(dir, 104);
    revisar(v == 0x00687F3A12345670ull, "E1: pack(0x7F3A12345670, 104) = 0x00687F3A12345670");
    revisar(peso(v) == 104 && direccion(v) == dir, "E2: peso = 104 y direccion = 0x7F3A12345670");
    uint64_t w = arrancar_ticket(v);
    revisar(peso(w) == 103 && direccion(w) == dir, "E3: tras arrancar un ticket, peso 103 y misma direccion");
    uint64_t a = pack19(dir, 300000);
    revisar((a >> 45) == 300000 && direccion19(a) == dir, "E4: con 19 bits caben 300000 tickets y la direccion vuelve");
    std::printf(fallos ? "\nTe faltan %d.\n" : "\nTodo correcto.\n", fallos);
    return 0;
}
