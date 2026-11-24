#!/usr/bin/env bash
# Todo lo que depende de los binarios oficiales fuera de Docker, en orden, para no medir dos cosas a la vez.
# Tarda unas 5 horas: 10 min de compilación, 2 h 50 min de «oficial», 30 min de «extra» y 1 h 20 min de «alineado».
# Si algo se corta, vuelve a lanzarlo: salta lo ya compilado y medido. Todo queda también en bin/pendientes.log.
# Uso: bash pendientes.sh [num_trabajos]
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p bin
exec > >(tee -a bin/pendientes.log) 2>&1
paso() { echo "=== $(date '+%F %H:%M') $*"; "$@"; }
paso bash preparar.sh "${1:-12}"
paso bash correr_nativo.sh oficial
paso bash correr_nativo.sh extra
paso bash correr_nativo.sh alineado
paso python3 analizar.py
echo "=== $(date '+%F %H:%M') PENDIENTES TERMINADOS"
