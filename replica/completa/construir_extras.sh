#!/usr/bin/env bash
# Se ejecuta DENTRO del contenedor, después de construir_en_docker.sh.
# Compila las herramientas propias contra las mismas librerías que los binarios del paper.
set -eu
cd /trabajo
gcc -O2 -shared -fPIC /herramientas/fijar_hilos.c -o fijar_hilos.so -ldl -pthread

# El paper deja a Williams fuera de la figura 6 y su línea viene comentada. La activamos para comprobar esa decisión.
sed -i 's|^//BENCH_PTR("JSS-OS"|BENCH_PTR("JSS-OS"|' atomic_shared_ptr/benchmark/bench_atomic_shared_ptrs.cpp
cp /herramientas/bench_basura.cpp atomic_shared_ptr/benchmark/bench_basura.cpp
grep -q 'add_benchmark(basura)' atomic_shared_ptr/benchmark/CMakeLists.txt || echo 'add_benchmark(basura)' >> atomic_shared_ptr/benchmark/CMakeLists.txt
(cd atomic_shared_ptr/build && cmake .. >/dev/null && make -j "${JOBS:-12}" bench_basura bench_atomic_shared_ptrs)

# El CMake del artefacto compila este binario sin -mcx16, y sin esa bandera el boost::atomic de 16 bytes de Williams cae en un
# candado (lock_pool). Williams se mide con este segundo binario; los demás no usan átomos de 16 bytes y siguen con el original.
mkdir -p atomic_shared_ptr/build_cx16
(cd atomic_shared_ptr/build_cx16
 cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-12 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-mcx16 \
   -DCMAKE_PREFIX_PATH="/trabajo/install/folly/lib/cmake/folly;/trabajo/install/fmt/lib/cmake/fmt;/trabajo/install/parlaylib/share/parlay/cmake/" \
   -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/trabajo/ext/googletest -DFETCHCONTENT_SOURCE_DIR_BENCHMARK=/trabajo/ext/benchmark -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
   -Dfolly_DIR=/trabajo/install/folly/lib/cmake/folly -Dfmt_DIR=/trabajo/install/fmt/lib/cmake/fmt -DParlay_DIR=/trabajo/install/parlaylib/share/parlay/cmake >/dev/null
 make -j "${JOBS:-12}" bench_atomic_shared_ptrs)
echo "EXTRAS TERMINADOS"
