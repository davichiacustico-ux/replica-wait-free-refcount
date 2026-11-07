#!/usr/bin/env bash
# Se ejecuta DENTRO del contenedor, con el artefacto montado en /trabajo.
# Sigue el README del artefacto oficial (scripts/{folly,parlay,throughput,latency}/install.sh) con tres diferencias:
# compila en paralelo, solo los dos binarios que usa el paper, y fija las versiones de Google Test y Google Benchmark.
set -eu
cd /trabajo
JOBS="${JOBS:-12}"

if [ ! -d install/folly/lib ]; then
  mkdir -p install
  (cd folly
   python3 ./build/fbcode_builder/getdeps.py --allow-system-packages --scratch-path=/trabajo/scratch --num-jobs="$JOBS" \
      build --no-tests --install-dir=/trabajo/install/folly)
  cp -r scratch/installed/. install/
  (cd install; for d in fmt-*; do mv "$d" fmt; done)
fi

if [ ! -d install/parlaylib/share ]; then
  mkdir -p install/parlaylib parlaylib/build
  (cd parlaylib/build && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=/trabajo/install/parlaylib .. && make -j "$JOBS" install)
fi

[ -d ext/googletest ] || git clone -q --depth 1 -b v1.14.0 https://github.com/google/googletest.git ext/googletest
[ -d ext/benchmark ]  || git clone -q --depth 1 -b v1.8.3  https://github.com/google/benchmark.git  ext/benchmark
EXT="-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/trabajo/ext/googletest -DFETCHCONTENT_SOURCE_DIR_BENCHMARK=/trabajo/ext/benchmark -DFETCHCONTENT_FULLY_DISCONNECTED=ON"
PFX="/trabajo/install/folly/lib/cmake/folly;/trabajo/install/fmt/lib/cmake/fmt;/trabajo/install/parlaylib/share/parlay/cmake/"

mkdir -p concurrent_deferred_rc/build
(cd concurrent_deferred_rc/build
 cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-12 -DCDRC_BENCHMARK=On -DCDRC_TEST=Off -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PFX" $EXT
 make -j "$JOBS" bench_stack)

# Único parche al código del paper: más valores de p, para no limitarnos a 1, 28 y 56
sed -i 's/->RangeMultiplier(28)->Range(1, 56);/->Arg(1)->Arg(2)->Arg(4)->Arg(8)->Arg(12)->Arg(16)->Arg(24)->Arg(28)->Arg(56);/' \
    atomic_shared_ptr/benchmark/bench_atomic_shared_ptrs.cpp
mkdir -p atomic_shared_ptr/build
(cd atomic_shared_ptr/build
 cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-12 -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PFX" $EXT \
   -Dfolly_DIR=/trabajo/install/folly/lib/cmake/folly -Dfmt_DIR=/trabajo/install/fmt/lib/cmake/fmt -DParlay_DIR=/trabajo/install/parlaylib/share/parlay/cmake
 make -j "$JOBS" bench_atomic_shared_ptrs)
echo "CONSTRUCCION TERMINADA"
