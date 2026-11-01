// Parte 2 · Bits, hexadecimal, desplazamientos y máscaras
// Lo mínimo para el paper (sección 3.2, figura 3): un puntero de 64 bits solo usa 48 para la
// dirección. Los 16 bits altos quedan libres y el paper guarda ahí el PESO (los tickets del cartel).
// pack junta dirección y peso en una palabra; unpack los separa; restar 2^48 quita un ticket.
#include <cstdint>    // uint64_t, uint16_t: enteros sin signo de tamaño exacto
#include <cstdio>

const uint64_t UN_TICKET   = uint64_t(1) << 48;  // 2^48 = pack(null, 1)
const uint64_t MASCARA_DIR = UN_TICKET - 1;      // 48 unos: 0x0000FFFFFFFFFFFF

uint64_t pack(uint64_t dir, uint64_t peso) { return (peso << 48) | dir; }  // peso arriba, dirección abajo
uint64_t peso(uint64_t v)      { return v >> 48; }           // quedarse con los 16 bits altos
uint64_t direccion(uint64_t v) { return v & MASCARA_DIR; }   // quedarse con los 48 bits bajos

int main() {
    // 1. Hexadecimal: cada dígito son 4 bits; %016llx imprime los 16 dígitos de una palabra
    uint64_t dir = 0x7F3A12345670;                // una dirección de 48 bits (12 dígitos hex)
    std::printf("dir          = 0x%016llx\n", (unsigned long long)dir);

    // 2. pack: el cartel es UNA palabra con (peso | dirección)
    uint64_t cartel = pack(dir, 104);
    std::printf("cartel       = 0x%016llx  (peso %llu, dir 0x%llx)\n",
                (unsigned long long)cartel, (unsigned long long)peso(cartel), (unsigned long long)direccion(cartel));

    // 3. load = arrancar un ticket: restar 2^48 solo cambia los bits altos
    cartel -= UN_TICKET;
    std::printf("tras un load = 0x%016llx  (peso %llu, la direccion no cambia)\n",
                (unsigned long long)cartel, (unsigned long long)peso(cartel));

    // 4. Aritmética que da la vuelta: los enteros sin signo trabajan módulo 2^n
    uint16_t w = 0;
    w = w - 1;                                    // en 16 bits: 0 - 1 = 65535
    std::printf("uint16_t 0 - 1 = %u\n", (unsigned)w);

    // 5. Un cartel nulo (dirección 0) al que se le resta un ticket: el peso da la vuelta,
    //    pero la dirección sigue siendo 0. Por eso los null no necesitan recarga (sección 5).
    uint64_t nulo = pack(0, 0) - UN_TICKET;
    std::printf("null - ticket = 0x%016llx (peso %llu, dir %llu)\n",
                (unsigned long long)nulo, (unsigned long long)peso(nulo), (unsigned long long)direccion(nulo));

    // 6. Un peso "negativo" sería catastrófico: -1 en 16 bits son 65535 tickets fantasma.
    return 0;
}
