#!/usr/bin/env bash
# Instrumenta una copia del código de los autores y mide el máximo de intentos de CAS.
# La copia se genera al vuelo en bin/ (no se versiona: su repositorio no declara licencia).
# Uso: bash correr_cota_cas.sh carpeta_de_salida [segundos]
set -euo pipefail
cd "$(dirname "$0")"
AUTORES=../../../codigo/autores/cpp/atomic_shared_ptr.h
BIN=../bin; SALIDA="$1"; SEG="${2:-3}"; mkdir -p "$BIN" "$SALIDA"

sed -e 's/^#define MAX_THREADS (1 << 16)/#define MAX_THREADS (COTA_P)/' \
    -e 's/^#define MAX_WEIGHT ((1UL << TAG_BITS) - 1)/#define MAX_WEIGHT (COTA_C)/' \
    -e 's/uint64_t current_weight = weight;/uint64_t current_weight = weight; CuentaCAS cuenta;/' \
    -e 's/size_t expected = pack<T>(control_block, current_weight);/cuenta.n++; size_t expected = pack<T>(control_block, current_weight);/' \
    "$AUTORES" > "$BIN/atomic_shared_ptr_instrumentado.h"
for marca in 'COTA_P' 'COTA_C' 'CuentaCAS cuenta' 'cuenta.n++'; do
  grep -q "$marca" "$BIN/atomic_shared_ptr_instrumentado.h" || { echo "no se pudo instrumentar: $marca"; exit 1; }
done

compilar() { g++ -std=c++20 -O2 -pthread -I"$BIN" -DCOTA_P="$1" -DCOTA_C="$2" cota_cas.cpp -o "$BIN/$3"; }
compilar '(1UL<<16)' '((1UL<<19)-1)' cota_real     # los valores del paper: P = 2^16, C = 2^19
compilar 32 64 cota_p32                             # P = 32, C = 2P: recarga cada ~32 lecturas
compilar 32 32 cota_p32_c32                         # incumple C >= 2P: variante rota a propósito

CSV="$SALIDA/cota_cas.csv"
echo "variante,hilos,pct_stores,P,C,operaciones,recargas,intentos,max_intentos,errores,i1,i2,i3,i4,i5,i6,i7,i8,codigo_salida" > "$CSV"
for variante in cota_real cota_p32; do
  for hilos in 2 4 8 16 24 32; do
    for pct in 0 1 10; do
      linea=$("$BIN/$variante" "$hilos" "$SEG" "$pct") && echo "$variante,$linea,0" >> "$CSV"
    done
  done
done
# La variante rota puede abortar por diseño: lo que interesa es justo eso, así que se anota el código de salida.
for hilos in 8 24; do
  set +e; linea=$("$BIN/cota_p32_c32" "$hilos" "$SEG" 1 2>/dev/null | tail -1); codigo=${PIPESTATUS[0]}; set -e
  echo "cota_p32_c32,${linea:-$hilos,1,32,32,,,,,,,,,,,,,},$codigo" >> "$CSV"
done
echo "listo: $CSV"
