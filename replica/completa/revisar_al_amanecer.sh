#!/usr/bin/env bash
# Lo lanza un temporizador a las 4:30, sin nadie delante. Si pendientes.sh aún corre, lo espera.
#   1. Comprueba los datos sin ninguna IA y lo deja en bin/comprobacion_final.txt.
#   2. Si todo está bien, hace una copia de seguridad y pide a un agente que siga TRASPASO.md: Claude si aún hay acceso,
#      y si no, agy. El agente no se lanza si los datos no están completos.
#   3. Comprueba que el agente no tocó datos, código ni archivos del equipo.
# Todo queda en bin/revision.log. El agente no hace commit ni push: el usuario revisa sus cambios.
set -uo pipefail
cd "$(dirname "$0")"
AQUI="$PWD"; RAIZ="$(cd ../.. && pwd)"; RES="$AQUI/../resultados/david-ultra9-285"
AGENTES="${AGENTES:-$HOME/.local/bin}"; ESPERA_MIN="${ESPERA_MIN:-180}"; INTENTOS=3; LIMITE=5400
RESUMEN="$AQUI/bin/resumen_agente.md"
mkdir -p bin
exec >>bin/revision.log 2>&1
aviso() { echo "=== $(date '+%F %H:%M') $*"; }

aviso empieza la revisión
for _ in $(seq "$ESPERA_MIN"); do pgrep -f 'pendientes\.sh' >/dev/null || break; sleep 60; done
python3 herramientas/comprobar_final.py > bin/comprobacion_final.txt 2>&1; codigo=$?
cat bin/comprobacion_final.txt
if [ "$codigo" != 0 ]; then aviso "los datos no están completos: no se lanza ningún agente"; exit 1; fi

rm -rf bin/respaldo && mkdir -p bin/respaldo
cp EXPLICACION.md README.md TRASPASO.md bin/respaldo/
cp -r "$RES" bin/respaldo/resultados
huella() { (cd "$RES" && find . -type f -name '*.csv' -print0 | sort -z | xargs -0 sha256sum) ; (cd "$AQUI" && sha256sum *.sh *.py herramientas/*); }
huella > bin/huella_antes.txt

read -r -d '' ENCARGO <<'T' || true
Lee replica/completa/TRASPASO.md y síguelo paso a paso. No hay nadie delante: no hagas preguntas; si dudas, elige lo prudente y anótalo.
La medición ya terminó y las tablas ya están regeneradas. El paso 1 ya está hecho: su resultado está en replica/completa/bin/comprobacion_final.txt.
Solo puedes editar replica/completa/EXPLICACION.md, replica/completa/README.md y replica/resultados/david-ultra9-285/LEEME.md. No cambies CSV, figuras ni código. No hagas commit ni push.
Puede que un intento anterior ya haya hecho parte del trabajo: mira los documentos antes de editarlos.
En la terminal usa solo comandos sueltos, sin encadenarlos, de esta lista: grep, wc, tail, head, ls, cat, sort, cut, python3, git status, git diff, git log. Cualquier otro comando corta tu sesión. Lee y edita los archivos con tus propias herramientas.
Cuando termines del todo, y solo entonces, escribe replica/completa/bin/resumen_agente.md con: qué cambiaste; la respuesta a las 7 preguntas, con la tabla de la que sale cada cifra; y las dudas que queden para el usuario.
T

PERMISOS='{"permissions":{"allow":["command(grep)","command(wc)","command(tail)","command(head)","command(ls)","command(cat)","command(sort)","command(cut)","command(python3)","command(git status)","command(git diff)","command(git log)"]}}'
HERRAMIENTAS='Read,Edit,Write,Glob,Grep,Bash(grep:*),Bash(wc:*),Bash(tail:*),Bash(head:*),Bash(ls:*),Bash(cat:*),Bash(sort:*),Bash(cut:*),Bash(python3:*),Bash(git status:*),Bash(git diff:*),Bash(git log:*)'

cd "$RAIZ"
rm -f "$RESUMEN"
aviso "probando con Claude"
timeout "$LIMITE" "$AGENTES/claude" -p "$ENCARGO" --permission-mode acceptEdits --allowedTools "$HERRAMIENTAS" | tail -20
for intento in $(seq "$INTENTOS"); do
  [ -s "$RESUMEN" ] && break
  aviso "probando con agy, intento $intento"
  # agy devuelve 0 aunque no haga nada, así que lo que cuenta es que exista el resumen
  "$AGENTES/agy" -p "$ENCARGO" --mode accept-edits --print-timeout "${LIMITE}s" --settings "$PERMISOS" | tail -20
done
cd "$AQUI"

if [ -s "$RESUMEN" ]; then aviso "el agente terminó"; else aviso "NINGÚN AGENTE TERMINÓ: la documentación puede estar a medias"; fi
huella > bin/huella_despues.txt
if cmp -s bin/huella_antes.txt bin/huella_despues.txt; then aviso "datos y código intactos"; else aviso "ATENCIÓN: cambiaron datos o código"; diff bin/huella_antes.txt bin/huella_despues.txt; fi
ajenos=$(git -C "$RAIZ" status --short -- codigo replica/informe)
if [ -z "$ajenos" ]; then aviso "codigo/ y replica/informe/ intactos"; else aviso "ATENCIÓN: cambiaron archivos del equipo"; echo "$ajenos"; fi
aviso "revisión terminada. Copia de seguridad en bin/respaldo/"
