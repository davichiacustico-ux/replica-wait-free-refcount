#!/usr/bin/env bash
# Dos controles para explicar por qué difieren las baterías con y sin Docker. Tarda unos 20 minutos.
#   alineacion: bench.cpp del equipo con y sin alignas(128) en Pila. codigo/bench.cpp no se toca: se parchea una copia en bin/.
#   docker:     el mismo código compilado y ejecutado dentro y fuera del contenedor, alternando.
# Antes hay que haber corrido correr_docker.sh (imagen) y correr_nativo.sh equipo (jemalloc y bench_equipo).
set -euo pipefail
cd "$(dirname "$0")"
AQUI="$PWD"; RAIZ_REPO="$(cd ../.. && pwd)"; RES="$AQUI/../resultados/david-ultra9-285"
DOCKER="${DOCKER:-sudo docker}"; IMAGEN=replica-wait-free; CONTENEDOR=replica-control
JEMALLOC="$AQUI/artefacto/raiz/usr/lib/x86_64-linux-gnu/libjemalloc.so.2"
BANDERAS=(-std=c++20 -O2 -pthread -mcx16 -Wno-deprecated-declarations)

# Copia alineada: el único cambio es «alignas(128)», el mismo valor que usa atomic_stack en el programa oficial
sed 's/^template <class I> struct Pila {/template <class I> struct alignas(128) Pila {/' "$RAIZ_REPO/codigo/bench.cpp" > bin/bench_alineado.cpp
grep -q 'alignas(128) Pila' bin/bench_alineado.cpp
sed 's|../../../codigo/bench.cpp|bench_alineado.cpp|' herramientas/bench_equipo.cpp > bin/bench_equipo_alineado.cpp
g++-12 "${BANDERAS[@]}" -I"$RAIZ_REPO/codigo" bin/bench_equipo_alineado.cpp -o bin/bench_equipo_alineado -latomic

# Contenedor que se queda abierto, para que entrar en él no cueste nada en cada medición
$DOCKER rm -f "$CONTENEDOR" >/dev/null 2>&1 || true
$DOCKER run -d --rm --name "$CONTENEDOR" --mount "type=bind,src=$RAIZ_REPO/codigo,dst=/r/codigo,readonly" \
       --mount "type=bind,src=$AQUI/herramientas,dst=/r/replica/completa/herramientas,readonly" \
       --mount "type=bind,src=$AQUI/bin,dst=/control" "$IMAGEN" sleep infinity >/dev/null
trap '$DOCKER rm -f "$CONTENEDOR" >/dev/null 2>&1 || true' EXIT
$DOCKER exec "$CONTENEDOR" g++-12 "${BANDERAS[@]}" /r/replica/completa/herramientas/bench_equipo.cpp -o /control/bench_equipo_docker -latomic

COMUN=(--base "$AQUI/bin/bench_equipo" --jemalloc "$JEMALLOC" --salida "$RES/controles.csv")
python3 herramientas/controles.py --control alineacion --otro "$AQUI/bin/bench_equipo_alineado" "${COMUN[@]}"
python3 herramientas/controles.py --control docker --otro /control/bench_equipo_docker --contenedor "$CONTENEDOR" --docker "$DOCKER" "${COMUN[@]}"
echo "CONTROLES TERMINADOS"
