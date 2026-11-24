#!/usr/bin/env bash
# ASan, UBSan y TSan sobre los rivales, que hasta ahora solo se habían pasado a Autores y a la versión ingenua.
#   bench.cpp (Autores, Libstdc++, Williams, Mutex): ASan + UBSan y TSan, sin Docker.
#   bench_stack oficial (los 5 del paper): ASan + UBSan dentro de Docker. El CMake del artefacto ya los activa en modo Debug.
#   No se pasa TSan al oficial: libfolly.a no está instrumentada y daría avisos falsos.
# detect_leaks=0 porque aquí se busca el uso de memoria ya liberada; la memoria retenida a propósito la mide bench_basura.
# Uso: bash sanitizers_rivales.sh archivo_de_salida         Tarda unos 10 minutos.
set -uo pipefail
cd "$(dirname "$0")"
AQUI="$PWD"; BIN=../bin; A="$(cd .. && pwd)/artefacto_docker"; SALIDA="$1"
DOCKER="${DOCKER:-sudo docker}"; IMAGEN=replica-wait-free
BANDERAS=(-std=c++20 -O1 -g -pthread -mcx16 -Wno-deprecated-declarations -fno-omit-frame-pointer)
export ASAN_OPTIONS=detect_leaks=0

probar() {  # etiqueta orden...
  local etiqueta="$1" texto cod
  shift
  texto=$("$@" 2>&1); cod=$?
  echo "$etiqueta: código $cod, $(grep -cE 'ERROR: AddressSanitizer|WARNING: ThreadSanitizer|runtime error' <<<"$texto") avisos, $(sed -n 's/.*Throughput = \([0-9.]*\) Mop.*/\1 Mop\/s/p' <<<"$texto" | head -1)"
  grep -E 'SUMMARY|runtime error' <<<"$texto" | sed 's/^.*\(replica-wait-free-refcount\|trabajo\)\///' | sort | uniq -c | sort -rn | head -5
}

{
  echo "# bench.cpp del equipo, con ASan + UBSan"
  g++-12 "${BANDERAS[@]}" -fsanitize=address,undefined bench_equipo.cpp -o "$BIN/bench_equipo_asan" -latomic
  for impl in Autores Libstdc++ Williams Mutex; do
    for punto in "1 50 8" "1 50 16" "10 10 16"; do
      set -- $punto
      probar "$impl n=$1 u=$2 hilos=$3" "$BIN/bench_equipo_asan" rendimiento "$impl" "$1" "$2" "$3" 4
    done
  done

  echo; echo "# bench.cpp del equipo, con TSan"
  g++-12 "${BANDERAS[@]}" -fsanitize=thread bench_equipo.cpp -o "$BIN/bench_equipo_tsan" -latomic
  for impl in Autores Libstdc++ Williams Mutex; do
    probar "$impl n=1 u=50 hilos=8" setarch -R "$BIN/bench_equipo_tsan" rendimiento "$impl" 1 50 8 4
  done

  echo; echo "# bench_stack oficial, con ASan + UBSan, dentro de Docker"
  $DOCKER run --rm --mount "type=bind,src=$A,dst=/trabajo" "$IMAGEN" bash -c '
    PFX="/trabajo/install/folly/lib/cmake/folly;/trabajo/install/fmt/lib/cmake/fmt;/trabajo/install/parlaylib/share/parlay/cmake/"
    EXT="-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/trabajo/ext/googletest -DFETCHCONTENT_SOURCE_DIR_BENCHMARK=/trabajo/ext/benchmark -DFETCHCONTENT_FULLY_DISCONNECTED=ON"
    mkdir -p /trabajo/concurrent_deferred_rc/build_asan && cd /trabajo/concurrent_deferred_rc/build_asan &&
    cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-12 -DCDRC_BENCHMARK=On -DCDRC_TEST=Off -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$PFX" $EXT >/dev/null &&
    make -j 12 bench_stack 2>&1 | tail -1'
  for alg in ivo gnu jss-free folly mine; do
    for hilos in 8 16; do
      probar "$alg n=1 u=50 hilos=$hilos" $DOCKER run --rm -e ASAN_OPTIONS=detect_leaks=0 -e NUM_THREADS=$hilos --mount "type=bind,src=$A,dst=/trabajo,readonly" "$IMAGEN" \
             /trabajo/concurrent_deferred_rc/build_asan/benchmarks/bench_stack --threads=$hilos --alg=$alg -r 4 -s 1 -u 50 -i 1 --peek=true
    done
  done
} 2>&1 | tee "$SALIDA"
