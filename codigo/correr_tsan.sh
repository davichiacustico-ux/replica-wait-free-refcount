#!/usr/bin/env bash
# Semana 1 (David): AddressSanitizer con 8/12/16 hilos y ThreadSanitizer. Uso: bash correr_tsan.sh [carpeta]
cd "$(dirname "$0")"; R="${1:-../replica/resultados/david-i5-10400}"; mkdir -p "$R" bin
g++ -std=c++20 -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread estres_asan.cpp -o bin/estres_asan
g++ -std=c++20 -O1 -g -fsanitize=thread -pthread estres_asan.cpp -o bin/estres_tsan
{ for h in 8 12 16; do echo "\$ ./bin/estres_asan autores $h 200000"; ./bin/estres_asan autores $h 200000 2>&1; done; } > "$R/03_asan_autores_hilos.txt"
{ for h in 8 16; do echo "\$ ./bin/estres_tsan autores $h 50000"; setarch -R ./bin/estres_tsan autores $h 50000 2>&1 | tail -5; done
  echo "\$ ./bin/estres_tsan ingenua 8 50000  (líneas resumidas)"
  setarch -R ./bin/estres_tsan ingenua 8 50000 2>&1 | grep -E 'version=|WARNING|SUMMARY|terminado' | sort | uniq -c | head -8; } > "$R/09_tsan.txt"
cat "$R/03_asan_autores_hilos.txt" "$R/09_tsan.txt"
