#!/usr/bin/env bash
# Prepara todo lo necesario para la réplica completa SIN sudo y sin tocar el sistema:
#   1. descarga el artefacto oficial del paper de Zenodo y comprueba su MD5;
#   2. descarga los .deb de las dependencias (sin instalarlos) y los extrae en artefacto/raiz/;
#   3. compila Folly, ParlayLib y los dos binarios oficiales del paper con g++-12.
# Uso: bash preparar.sh [num_trabajos]      Tarda unos 20 minutos.
set -euo pipefail
cd "$(dirname "$0")"
AQUI="$PWD"; A="$AQUI/artefacto"; RAIZ="$A/raiz"; JOBS="${1:-12}"
ZIP_MD5=e7e03425dfd426385a769788e5460fcd
ZIP_URL=https://zenodo.org/api/records/11775182/files/Euro-PAR_2024_paper_432.zip/content

# 1. Artefacto oficial ---------------------------------------------------------------------------------------------
mkdir -p "$A"
if [ ! -d "$A/concurrent_deferred_rc" ]; then
  curl -sSL -o "$A/art.zip" "$ZIP_URL"
  echo "$ZIP_MD5  $A/art.zip" | md5sum -c -
  unzip -q -o "$A/art.zip" -d "$A"
fi

# 2. Dependencias como carpeta local -------------------------------------------------------------------------------
PAQUETES="libboost1.83-dev
libboost-atomic1.83-dev libboost-atomic1.83.0 libboost-chrono1.83-dev libboost-chrono1.83.0t64
libboost-context1.83-dev libboost-context1.83.0 libboost-date-time1.83-dev libboost-date-time1.83.0
libboost-filesystem1.83-dev libboost-filesystem1.83.0 libboost-program-options1.83-dev libboost-program-options1.83.0
libboost-regex1.83-dev libboost-regex1.83.0 libboost-system1.83-dev libboost-system1.83.0
libboost-thread1.83-dev libboost-thread1.83.0
libdouble-conversion-dev libdouble-conversion3 libgflags-dev libgflags2.2 libgoogle-glog-dev libgoogle-glog0v6t64
libfmt-dev libfmt9 libssl-dev libjemalloc-dev libjemalloc2 libunwind-dev libunwind8 liblzma-dev zlib1g-dev"
if [ ! -d "$RAIZ/usr" ]; then
  mkdir -p "$A/debs" "$RAIZ"
  (cd "$A/debs" && apt-get download $PAQUETES)
  for d in "$A"/debs/*.deb; do dpkg -x "$d" "$RAIZ"; done
fi
[ "${SOLO_DESCARGA:-0}" = 1 ] && { echo "descarga lista"; exit 0; }

export CC=gcc-12 CXX=g++-12
export CMAKE_PREFIX_PATH="$RAIZ/usr"
export LD_LIBRARY_PATH="$RAIZ/usr/lib/x86_64-linux-gnu:$A/install/folly/lib:${LD_LIBRARY_PATH:-}"
export CPATH="$RAIZ/usr/include:$RAIZ/usr/include/x86_64-linux-gnu"
export LIBRARY_PATH="$RAIZ/usr/lib/x86_64-linux-gnu"
COMUN=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$RAIZ/usr" -DCMAKE_LIBRARY_ARCHITECTURE=x86_64-linux-gnu)

# 3. Folly, en el commit exacto que trae el artefacto --------------------------------------------------------------
if [ ! -f "$A/install/folly/lib/libfolly.a" ]; then
  cmake -S "$A/folly" -B "$A/build_folly" "${COMUN[@]}" -DCMAKE_INSTALL_PREFIX="$A/install/folly" \
        -DBUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_CXX_STANDARD=20
  cmake --build "$A/build_folly" -j "$JOBS" --target install
fi
# los CMakeLists del artefacto buscan fmt en install/fmt
mkdir -p "$A/install/fmt/lib/cmake"
ln -sfn "$RAIZ/usr/include" "$A/install/fmt/include"
ln -sfn "$RAIZ/usr/lib/x86_64-linux-gnu/cmake/fmt" "$A/install/fmt/lib/cmake/fmt"
# fmt-targets.cmake sube 4 carpetas para hallar su raíz: desde install/fmt/lib/cmake/fmt llega a install/ y busca ahí lib e include
ln -sfn "$RAIZ/usr/lib" "$A/install/lib"
ln -sfn "$RAIZ/usr/include" "$A/install/include"

# 4. ParlayLib -----------------------------------------------------------------------------------------------------
if [ ! -d "$A/install/parlaylib/share" ]; then
  cmake -S "$A/parlaylib" -B "$A/build_parlay" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$A/install/parlaylib"
  cmake --build "$A/build_parlay" -j "$JOBS" --target install
fi

# 5. Google Test y Google Benchmark: versiones fijas, porque los CMakeLists piden la rama «main» de hoy ---------------
[ -d "$A/ext/googletest" ] || git clone -q --depth 1 -b v1.14.0 https://github.com/google/googletest.git "$A/ext/googletest"
[ -d "$A/ext/benchmark" ]  || git clone -q --depth 1 -b v1.8.3  https://github.com/google/benchmark.git  "$A/ext/benchmark"
EXT=(-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST="$A/ext/googletest" -DFETCHCONTENT_SOURCE_DIR_BENCHMARK="$A/ext/benchmark" -DFETCHCONTENT_FULLY_DISCONNECTED=ON)

# 6. Binario de rendimiento (figura 5) -----------------------------------------------------------------------------
cmake -S "$A/concurrent_deferred_rc" -B "$A/concurrent_deferred_rc/build" "${COMUN[@]}" "${EXT[@]}" -DCDRC_BENCHMARK=On -DCDRC_TEST=Off
cmake --build "$A/concurrent_deferred_rc/build" -j "$JOBS" --target bench_stack

# 7. Binario de latencia (figura 6). Único parche: más valores de p, para no limitarnos a 1, 28 y 56 ------------------
sed -i 's/->RangeMultiplier(28)->Range(1, 56);/->Arg(1)->Arg(2)->Arg(4)->Arg(8)->Arg(12)->Arg(16)->Arg(24)->Arg(28)->Arg(56);/' \
    "$A/atomic_shared_ptr/benchmark/bench_atomic_shared_ptrs.cpp"
# El paper deja a Williams fuera de la figura 6 y su línea viene comentada. La activamos para comprobar esa decisión.
sed -i 's|^//BENCH_PTR("JSS-OS"|BENCH_PTR("JSS-OS"|' "$A/atomic_shared_ptr/benchmark/bench_atomic_shared_ptrs.cpp"
cmake -S "$A/atomic_shared_ptr" -B "$A/atomic_shared_ptr/build" "${COMUN[@]}" "${EXT[@]}" \
      -Dfolly_DIR="$A/install/folly/lib/cmake/folly" -Dfmt_DIR="$RAIZ/usr/lib/x86_64-linux-gnu/cmake/fmt" \
      -DParlay_DIR="$A/install/parlaylib/share/parlay/cmake"
cmake --build "$A/atomic_shared_ptr/build" -j "$JOBS" --target bench_atomic_shared_ptrs
# Sin -mcx16, el boost::atomic de 16 bytes de Williams cae en un candado (lock_pool). Williams se mide con este segundo binario.
cmake -S "$A/atomic_shared_ptr" -B "$A/atomic_shared_ptr/build_cx16" "${COMUN[@]}" "${EXT[@]}" -DCMAKE_CXX_FLAGS=-mcx16 \
      -Dfolly_DIR="$A/install/folly/lib/cmake/folly" -Dfmt_DIR="$RAIZ/usr/lib/x86_64-linux-gnu/cmake/fmt" \
      -DParlay_DIR="$A/install/parlaylib/share/parlay/cmake"
cmake --build "$A/atomic_shared_ptr/build_cx16" -j "$JOBS" --target bench_atomic_shared_ptrs

# 8. Prueba de memoria retenida, contra las mismas librerías ------------------------------------------------------
cp "$AQUI/herramientas/bench_basura.cpp" "$A/atomic_shared_ptr/benchmark/bench_basura.cpp"
grep -q 'add_benchmark(basura)' "$A/atomic_shared_ptr/benchmark/CMakeLists.txt" || echo 'add_benchmark(basura)' >> "$A/atomic_shared_ptr/benchmark/CMakeLists.txt"
cmake "$A/atomic_shared_ptr/build" >/dev/null
cmake --build "$A/atomic_shared_ptr/build" -j "$JOBS" --target bench_basura
echo "PREPARACION TERMINADA"
