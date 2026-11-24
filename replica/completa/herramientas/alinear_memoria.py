#!/usr/bin/env python3
"""¿Por qué alinear las pilas frena con n = 10⁶? Mide la RAM con y sin alignas(128), alternando las dos versiones.

Sin alinear, una pila de bench.cpp ocupa 8 B (Autores), 16 B (Libstdc++ y Williams) o 56 B (Mutex); alineada, 128 B.
Con n = 10⁶ el arreglo de pilas pasa de 8-16 MB, que caben en la L3 de 36 MB, a 128 MB, que no caben.
Si esa es la causa:
  con n = 10⁶, la versión alineada mueve más bytes por operación por la RAM;
  con n = 10⁵ (12,8 MB alineado, cabe) alinear no frena;
  el Mutex apenas cambia, porque sus 56 MB ya no cabían sin alinear.
Escribe <resultados>/nativo_alineado/memoria_alineado.csv.
Necesita sudo sin contraseña, bin/bench_equipo y bin/bench_equipo_alineado (correr_nativo.sh equipo y alineado).
Con la máquina libre. Tarda unos 10 minutos.
"""
import csv
import random
import re
import subprocess

from medir_memoria import AQUI, CONTADOR, con_contadores

SALIDA = AQUI.parent / "resultados" / "david-ultra9-285" / "nativo_alineado" / "memoria_alineado.csv"
VERSIONES = {"sin_alinear": AQUI / "bin" / "bench_equipo", "alineado": AQUI / "bin" / "bench_equipo_alineado"}
IMPLS = ["Autores", "Libstdc++", "Williams", "Mutex"]
TAMANOS = [100000, 1000000]
HILOS = 24
RONDAS = 3
SEGUNDOS = 10
CAMPOS = ["impl", "n", "version", "ronda", "mops", "gbs_lectura", "gbs_escritura", "bytes_por_op"]


def main():
    subprocess.run(["gcc", "-O2", str(AQUI / "herramientas" / "contar_imc.c"), "-o", str(CONTADOR)], check=True)
    with open(SALIDA, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, CAMPOS)
        w.writeheader()
        for ronda in range(1, RONDAS + 1):
            puntos = [(impl, n) for impl in IMPLS for n in TAMANOS]
            random.Random(ronda).shuffle(puntos)
            for impl, n in puntos:
                # Las dos versiones de un mismo punto van seguidas y en orden al azar, para que las compare el mismo estado de la máquina.
                versiones = list(VERSIONES)
                random.Random(f"{ronda}-{impl}-{n}").shuffle(versiones)
                for version in versiones:
                    orden = [str(VERSIONES[version]), "rendimiento", impl, str(n), "50", str(HILOS), str(SEGUNDOS)]
                    salida, leido, escrito = con_contadores(orden)
                    mops = float(re.search(r"Throughput = (\S+) Mop/s", salida).group(1))
                    fila = dict(impl=impl, n=n, version=version, ronda=ronda, mops=mops, gbs_lectura=f"{leido:.2f}",
                                gbs_escritura=f"{escrito:.2f}", bytes_por_op=f"{1000 * (leido + escrito) / mops:.0f}")
                    w.writerow(fila)
                    f.flush()
                    print(*fila.values(), flush=True)
    print("ALINEADO TERMINADO", flush=True)


if __name__ == "__main__":
    main()
