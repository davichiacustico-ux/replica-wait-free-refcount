#!/usr/bin/env python3
"""Comprueba que pendientes.sh terminó bien. No necesita ninguna IA: lee el registro y cuenta filas.

Uso: python3 herramientas/comprobar_final.py      Devuelve 0 si todo está completo y sin fallos.
"""
import csv
import re
import sys
from collections import Counter
from pathlib import Path

AQUI = Path(__file__).resolve().parent.parent
RES = AQUI.parent / "resultados" / "david-ultra9-285"
REGISTRO = AQUI / "bin" / "pendientes.log"
ESPERADO = [
    ("nativo_oficial/rendimiento.csv", "experimento", {"fig5": 1350, "afinidad": 1350, "pares": 700}),
    ("nativo_oficial/latencia.csv", "experimento", {"fig6": 675, "latafin": 585}),
    ("nativo_alineado/rendimiento.csv", "experimento", {"fig5": 1080, "afinidad": 1080}),
]
TOTALES = [("nativo_oficial/basura.csv", 75), ("nativo_oficial/paridad.csv", 30)]
FIGURAS = ["nativo_oficial_fig5.png", "nativo_oficial_fig6.png", "docker_frente_a_nativo_oficial.png",
           "nativo_oficial_dosis_respuesta.png", "nativo_oficial_memoria_retenida.png", "nativo_alineado_fig5.png"]


def leer(nombre):
    ruta = RES / nombre
    if not ruta.exists():
        return None
    with open(ruta, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def main():
    problemas = []
    texto = REGISTRO.read_text(encoding="utf-8", errors="replace") if REGISTRO.exists() else ""
    if "PENDIENTES TERMINADOS" not in texto:
        problemas.append("el registro no dice PENDIENTES TERMINADOS")
    fallos = re.findall(r"^.*(?:FALLO|sin datos|Traceback).*$", texto, re.M)
    print(f"Líneas de fallo en el registro: {len(fallos)}")
    print(*[f"  {l[:150]}" for l in fallos[:10]], sep="\n") if fallos else None
    if fallos:
        problemas.append(f"{len(fallos)} líneas de fallo en el registro")

    for nombre, campo, esperado in ESPERADO:
        filas = leer(nombre)
        if filas is None:
            problemas.append(f"falta {nombre}")
            continue
        cuenta = Counter(r[campo] for r in filas)
        malos = [r for r in filas if r["codigo"].strip() != "0"]
        vacios = [r for r in filas if "mops" in r and r["mops"] == ""]
        temps = [float(r["temp_c"]) for r in filas if r["temp_c"]]
        print(f"{nombre}: {dict(cuenta)}; esperado {esperado}; código distinto de 0: {len(malos)}; sin valor: {len(vacios)}; "
              f"a 90 °C o más: {sum(t >= 90 for t in temps)}; máxima {max(temps, default=0):.0f} °C")
        if dict(cuenta) != esperado:
            problemas.append(f"{nombre} no tiene las filas esperadas")
        if malos or vacios:
            problemas.append(f"{nombre} tiene mediciones fallidas")
    for nombre, esperado in TOTALES:
        filas = leer(nombre)
        print(f"{nombre}: {'no existe' if filas is None else len(filas)}; esperado {esperado}")
        if filas is None or len(filas) != esperado:
            problemas.append(f"{nombre} no tiene las filas esperadas")
    faltan = [f for f in FIGURAS if not (RES / "figuras" / f).exists()]
    print(f"Figuras nuevas que faltan: {faltan or 'ninguna'}")
    if faltan:
        problemas.append("faltan figuras")

    print("\nRESULTADO:", "todo completo y sin fallos" if not problemas else "HAY PROBLEMAS")
    print(*[f"- {p}" for p in problemas], sep="\n") if problemas else None
    return 1 if problemas else 0


if __name__ == "__main__":
    sys.exit(main())
