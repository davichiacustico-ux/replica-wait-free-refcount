#!/usr/bin/env bash
# ¿De dónde sale el hundimiento de Williams entre clústeres? El programa oficial lo tiene y bench.cpp no.
# Los dos usan la misma librería con una diferencia: el artefacto cambió std::atomic por boost::atomic.
# Aquí se mide bench.cpp dentro de Docker con las dos cabeceras, en pares de núcleos y con todos los núcleos.
# Antes hay que haber corrido controles.sh, que deja bin/bench_equipo_docker. Tarda unos 2 minutos.
set -euo pipefail
cd "$(dirname "$0")"
AQUI="$PWD"; RAIZ_REPO="$(cd ../.. && pwd)"; SALIDA="$AQUI/../resultados/david-ultra9-285/control_williams.csv"
DOCKER="${DOCKER:-sudo docker}"; IMAGEN=replica-wait-free
JEMALLOC=/usr/lib/x86_64-linux-gnu/libjemalloc.so

# Variante con la cabecera del artefacto. La copia de bench.cpp va en bin/ para que su «williams/...» no encuentre la del equipo
mkdir -p bin/inc/williams
cp artefacto_docker/atomic_shared_ptr/benchmark/external/anthonywilliams/atomic_shared_ptr bin/inc/williams/
cp "$RAIZ_REPO/codigo/bench.cpp" bin/bench_boost.cpp
sed 's|../../../codigo/bench.cpp|bench_boost.cpp|' herramientas/bench_equipo.cpp > bin/bench_equipo_boost.cpp
$DOCKER run --rm --mount "type=bind,src=$AQUI/bin,dst=/control" --mount "type=bind,src=$RAIZ_REPO/codigo,dst=/r/codigo,readonly" "$IMAGEN" \
       g++-12 -std=c++20 -O2 -pthread -mcx16 -Wno-deprecated-declarations -I/control/inc -I/r/codigo \
       /control/bench_equipo_boost.cpp -o /control/bench_equipo_boost -latomic

medir() {  # variante binario par n u hilos ronda
  local precarga="$JEMALLOC" fijar=()
  if [ "$3" != sistema ]; then precarga="$JEMALLOC:/fijar_hilos.so"; fijar=(-e "FIJAR_CPUS=$3" -e FIJAR_PRINCIPAL=7); fi
  mops=$($DOCKER run --rm --cap-add SYS_NICE --mount "type=bind,src=$AQUI/bin,dst=/control,readonly" \
         --mount "type=bind,src=$AQUI/artefacto_docker/fijar_hilos.so,dst=/fijar_hilos.so,readonly" \
         -e "LD_PRELOAD=$precarga" "${fijar[@]}" "$IMAGEN" "/control/$2" rendimiento Williams "$4" "$5" "$6" 2 |
         sed -n 's/.*Throughput = \([0-9.]*\) Mop.*/\1/p')
  echo "$1,\"$3\",$4,$5,$6,$7,$mops" >> "$SALIDA"
}

echo "variante,par,n,u,hilos,ronda,mops" > "$SALIDA"
for ronda in 1 2 3; do
  for variante in "std::atomic bench_equipo_docker" "boost::atomic bench_equipo_boost"; do
    set -- $variante
    for par in 8,9 8,12 9,13 16,20; do medir "$1" "$2" "$par" 1 50 2 "$ronda"; done
    for u in 1 10 50; do medir "$1" "$2" sistema 1 "$u" 24 "$ronda"; done
  done
done
cat "$SALIDA"
