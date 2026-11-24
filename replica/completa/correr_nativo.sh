#!/usr/bin/env bash
# Baterías SIN Docker, directamente sobre el sistema.
#   bash correr_nativo.sh equipo    Autores, Libstdc++, Williams y Mutex con codigo/bench.cpp del equipo.
#                                   Solo necesita g++ y jemalloc (preparar.sh con SOLO_DESCARGA=1 lo deja en artefacto/raiz).
#   bash correr_nativo.sh oficial   Las 5 implementaciones del paper con sus binarios oficiales.
#                                   Antes hay que compilarlos: bash preparar.sh
#   bash correr_nativo.sh extra     Más rondas donde el resultado salió inestable: latencia (15) y pares de núcleos (10).
#                                   Añade filas a nativo_oficial/ sin repetir lo medido. Después de «oficial».
#   bash correr_nativo.sh alineado  Rendimiento y afinidad de «equipo» con alignas(128) en Pila, para quitar la compartición
#                                   falsa de los paneles n = 10. codigo/bench.cpp no se toca: se parchea una copia en bin/.
set -euo pipefail
cd "$(dirname "$0")"
AQUI="$PWD"; MODO="${1:-equipo}"; RES="$AQUI/../resultados/david-ultra9-285"; A="$AQUI/artefacto"
LIBS="$A/raiz/usr/lib/x86_64-linux-gnu"
JEMALLOC="$LIBS/libjemalloc.so.2"
OFICIAL=(--motor oficial --raiz "$A" --shim "$AQUI/bin/fijar_hilos.so" --jemalloc "$JEMALLOC" --biblios "$LIBS:$A/install/folly/lib")
BANDERAS=(-std=c++20 -O2 -pthread -mcx16 -Wno-deprecated-declarations)
mkdir -p bin
gcc -O2 -shared -fPIC herramientas/fijar_hilos.c -o bin/fijar_hilos.so -ldl -pthread

case "$MODO" in
  equipo)
    g++-12 "${BANDERAS[@]}" herramientas/bench_equipo.cpp -o bin/bench_equipo -latomic
    python3 baterias.py --motor equipo --equipo "$AQUI/bin/bench_equipo" --shim "$AQUI/bin/fijar_hilos.so" \
            --jemalloc "$JEMALLOC" --salida "$RES/nativo" --solo fig5,fig6,afinidad,latafin,pares
    ;;
  oficial)
    python3 baterias.py "${OFICIAL[@]}" --salida "$RES/nativo_oficial"
    ;;
  extra)
    python3 baterias.py "${OFICIAL[@]}" --salida "$RES/nativo_oficial" --solo fig6,pares --rondas-fig5 15 --rondas-afinidad 10
    ;;
  alineado)
    sed 's/^template <class I> struct Pila {/template <class I> struct alignas(128) Pila {/' ../../codigo/bench.cpp > bin/bench_alineado.cpp
    grep -q 'alignas(128) Pila' bin/bench_alineado.cpp
    sed 's|../../../codigo/bench.cpp|bench_alineado.cpp|' herramientas/bench_equipo.cpp > bin/bench_equipo_alineado.cpp
    g++-12 "${BANDERAS[@]}" -I../../codigo bin/bench_equipo_alineado.cpp -o bin/bench_equipo_alineado -latomic
    python3 baterias.py --motor equipo --equipo "$AQUI/bin/bench_equipo_alineado" --shim "$AQUI/bin/fijar_hilos.so" \
            --jemalloc "$JEMALLOC" --salida "$RES/nativo_alineado" --solo fig5,afinidad
    ;;
  *) echo "uso: bash correr_nativo.sh equipo|oficial|extra|alineado"; exit 2 ;;
esac
