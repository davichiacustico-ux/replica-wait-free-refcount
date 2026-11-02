#!/usr/bin/env bash
# Ejecuta en Linux (WSL) todas las pruebas de la exposición y guarda cada salida como «captura de terminal» en ../capturas.
# Uso: cd codigo && bash ejecutar_pruebas.sh [carpeta_de_salida]
set -u
cd "$(dirname "$0")"
CAP="${1:-../capturas}"; mkdir -p "$CAP" bin   # carpeta de salida opcional, p. ej. ../replica/resultados/david-i5-10400
cap() { local f="$CAP/$1"; shift; { for c in "$@"; do echo "\$ $c"; eval "$c" 2>&1; echo; done; } > "$f"; echo "→ $f"; }

cap 00_entorno.txt "uname -sr" "g++ --version | head -1" "nproc" "lscpu | grep -E 'Model name|^CPU\(s\)'"

# 1. El código de los autores tal cual (demo de su repositorio, commit 8540c58)
cap 01_autores_demo.txt "g++ -std=c++20 -O2 autores/cpp/main.cpp -o bin/demo" "./bin/demo | tail -6"

# 2. La carrera con AddressSanitizer: versión ingenua frente al código de los autores
g++ -std=c++20 -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread estres_asan.cpp -o bin/estres_asan
cap 02_asan_ingenua.txt "./bin/estres_asan ingenua 2>&1 | grep -E 'version=|ERROR|WRITE|READ|#[0-3] |freed by|previously allocated|SUMMARY' | head -16"
cap 03_asan_autores.txt "./bin/estres_asan autores" "./bin/estres_asan autores 12 200000" "./bin/estres_asan autores 16 200000"
g++ -std=c++20 -O1 -g -fsanitize=thread -pthread estres_asan.cpp -o bin/estres_tsan
cap 09_tsan.txt "./bin/estres_tsan autores 8 50000" "./bin/estres_tsan autores 16 50000" "./bin/estres_tsan ingenua 8 50000 2>&1 | grep -E 'version=|WARNING|SUMMARY|terminado' | head -6"

# 3. El boceto didáctico (explicación, Parte 11) y dos variantes rotas
g++ -std=c++17 -O2 -pthread boceto/prueba.cpp -o bin/boceto
g++ -std=c++17 -O2 -pthread -DSIN_SALIDA -DSOLO_PAPER boceto/prueba.cpp -o bin/boceto_sin_salida
g++ -std=c++17 -O2 -pthread -DVAR_C=8 -DSOLO_PAPER boceto/prueba.cpp -o bin/boceto_c_igual_p
cap 04_boceto.txt "./bin/boceto"
cap 05_boceto_variantes.txt "./bin/boceto_sin_salida" "./bin/boceto_c_igual_p"

# 4. Mini-reproducción de los experimentos (sección 6)
g++ -std=c++20 -O2 -pthread bench.cpp -o bin/bench
./bin/bench rendimiento 1.0 > "$CAP/rendimiento.csv" 2> "$CAP/06_rendimiento.txt"
./bin/bench latencia 100000 > "$CAP/latencia.csv" 2> "$CAP/07_latencia.txt"
echo "listo"
