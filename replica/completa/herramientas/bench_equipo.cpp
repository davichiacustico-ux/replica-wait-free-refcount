// bench_equipo.cpp — línea de órdenes para codigo/bench.cpp, sin modificarlo.
// bench.cpp fija sus hilos y paneles dentro de su main; aquí se reutilizan sus plantillas para poder
// lanzar un solo punto y fijarlo a núcleos desde fuera, igual que los binarios oficiales.
//   ./bench_equipo rendimiento <impl> <n> <u> <hilos> <segundos>
//   ./bench_equipo latencia    <impl> <p> <muestras>
// impl: Autores, Libstdc++, Williams, Mutex
#define main main_de_bench
#include "../../../codigo/bench.cpp"
#undef main

template <class I> static void un_punto(int argc, char** argv) {
    std::string modo = argv[1];
    if (modo == "rendimiento") {
        long n = std::atol(argv[3]);
        int u = std::atoi(argv[4]), hilos = std::atoi(argv[5]);
        std::printf("Total Throughput = %.3f Mop/s\n", rendimiento<I>(hilos, n, u, std::atof(argv[6])));
        return;
    }
    int p = std::atoi(argv[3]), muestras = argc > 4 ? std::atoi(argv[4]) : 100000;
    double nspt = ns_por_tick();
    std::vector<double> vacio;
    for (int i = 0; i < 20000; i++) { uint64_t a = tsc(), b = tsc(); vacio.push_back((b - a) * nspt); }
    double sc = percentil(vacio, 0.5), scmin = *std::min_element(vacio.begin(), vacio.end());
    fila_latencia<I>(p, muestras, nspt, sc, scmin);
}

int main(int argc, char** argv) {
    if (argc < 4) { std::fputs("uso: ver cabecera de bench_equipo.cpp\n", stderr); return 2; }
    std::string impl = argv[2];
    if (impl == "Autores") un_punto<Autores>(argc, argv);
    else if (impl == "Libstdc++") un_punto<Libstdcpp>(argc, argv);
    else if (impl == "Williams") un_punto<Williams>(argc, argv);
    else if (impl == "Mutex") un_punto<Mutex>(argc, argv);
    else return 2;
}
