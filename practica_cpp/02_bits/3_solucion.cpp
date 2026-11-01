// Parte 2 · Solución
#include <cstdint>
#include <cstdio>

// E1
uint64_t pack(uint64_t dir, uint64_t peso) {
    return (peso << 48) | dir;                 // desplazar el peso 48 bits y combinar con OR
}

// E2
uint64_t peso(uint64_t v) {
    return v >> 48;                            // los 16 bits altos
}
uint64_t direccion(uint64_t v) {
    return v & ((uint64_t(1) << 48) - 1);      // máscara con 48 unos
}

// E3
uint64_t arrancar_ticket(uint64_t v) {
    return v - (uint64_t(1) << 48);            // restar 2^48 = -pack(null, 1): solo baja el peso
}

// E4
uint64_t pack19(uint64_t dir, uint64_t peso) {
    return (peso << 45) | (dir >> 3);          // 45 bits de dirección (sin los 3 ceros) y 19 de peso
}
uint64_t direccion19(uint64_t v) {
    return (v & ((uint64_t(1) << 45) - 1)) << 3;   // multiplicar por 8 devuelve los 3 ceros
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
