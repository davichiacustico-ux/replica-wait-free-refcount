#!/usr/bin/env bash
# Baterías SIN Docker, directamente sobre el sistema.
#   bash correr_nativo.sh equipo    Autores, Libstdc++, Williams y Mutex con codigo/bench.cpp del equipo.
#                                   Solo necesita g++ y jemalloc (preparar.sh con SOLO_DESCARGA=1 lo deja en artefacto/raiz).
#   bash correr_nativo.sh oficial   Las 5 implementaciones del paper con sus binarios oficiales.
#                                   Antes hay que compilarlos: bash preparar.sh
set -euo pipefail
cd "$(dirname "$0")"
AQUI="$PWD"; MODO="${1:-equipo}"; RES="$AQUI/../resultados/david-ultra9-285"; A="$AQUI/artefacto"
LIBS="$A/raiz/usr/lib/x86_64-linux-gnu"
JEMALLOC="$LIBS/libjemalloc.so.2"
mkdir -p bin
gcc -O2 -shared -fPIC herramientas/fijar_hilos.c -o bin/fijar_hilos.so -ldl -pthread

case "$MODO" in
  equipo)
    g++-12 -std=c++20 -O2 -pthread -mcx16 -Wno-deprecated-declarations herramientas/bench_equipo.cpp -o bin/bench_equipo -latomic
    python3 baterias.py --motor equipo --equipo "$AQUI/bin/bench_equipo" --shim "$AQUI/bin/fijar_hilos.so" \
            --jemalloc "$JEMALLOC" --salida "$RES/nativo" --solo fig5,fig6,afinidad,latafin,pares
    ;;
  oficial)
    python3 baterias.py --motor oficial --raiz "$A" --shim "$AQUI/bin/fijar_hilos.so" --jemalloc "$JEMALLOC" \
            --biblios "$LIBS:$A/install/folly/lib" --salida "$RES/nativo_oficial"
    ;;
  *) echo "uso: bash correr_nativo.sh equipo|oficial"; exit 2 ;;
esac
