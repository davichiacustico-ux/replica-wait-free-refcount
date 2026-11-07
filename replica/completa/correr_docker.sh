#!/usr/bin/env bash
# Baterías CON Docker: las 5 implementaciones del paper, con sus binarios oficiales.
# Baja el artefacto de Zenodo, construye la imagen y lo compila todo dentro del contenedor; luego mide.
# Lo ya compilado o medido se salta, así que se puede relanzar si algo se corta.
# Uso: bash correr_docker.sh [experimentos]    p. ej. bash correr_docker.sh pares     Sin argumento, todos (unas 3 h).
set -euo pipefail
cd "$(dirname "$0")"
AQUI="$PWD"; A="$AQUI/artefacto_docker"; RES="$AQUI/../resultados/david-ultra9-285/docker"
DOCKER="${DOCKER:-sudo docker}"; IMAGEN=replica-wait-free
ZIP_MD5=e7e03425dfd426385a769788e5460fcd
ZIP_URL=https://zenodo.org/api/records/11775182/files/Euro-PAR_2024_paper_432.zip/content
MONTAJE_TRABAJO="type=bind,src=$A,dst=/trabajo"

$DOCKER image inspect "$IMAGEN" >/dev/null 2>&1 || $DOCKER build -t "$IMAGEN" .

mkdir -p "$A" "$RES"
if [ ! -d "$A/concurrent_deferred_rc" ]; then
  curl -sSL -o "$A/art.zip" "$ZIP_URL"
  echo "$ZIP_MD5  $A/art.zip" | md5sum -c -
  unzip -q -o "$A/art.zip" -d "$A"
fi
if [ ! -x "$A/atomic_shared_ptr/build/benchmark/bench_basura" ]; then
  $DOCKER run --rm --mount "$MONTAJE_TRABAJO" --mount "type=bind,src=$AQUI/construir_en_docker.sh,dst=/construir.sh,readonly" \
         "$IMAGEN" bash /construir.sh
fi
if [ ! -x "$A/atomic_shared_ptr/build_cx16/benchmark/bench_atomic_shared_ptrs" ]; then
  $DOCKER run --rm --mount "$MONTAJE_TRABAJO" --mount "type=bind,src=$AQUI/herramientas,dst=/herramientas,readonly" \
         --mount "type=bind,src=$AQUI/construir_extras.sh,dst=/construir_extras.sh,readonly" "$IMAGEN" bash /construir_extras.sh
fi

$DOCKER run --rm --name replica-baterias --cap-add SYS_NICE --mount "$MONTAJE_TRABAJO" \
       --mount "type=bind,src=$AQUI/baterias.py,dst=/baterias.py,readonly" --mount "type=bind,src=$RES,dst=/resultados" \
       "$IMAGEN" python3 /baterias.py --motor oficial --salida /resultados ${1:+--solo "$1"}
