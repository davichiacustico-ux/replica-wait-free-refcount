#!/usr/bin/env python3
"""Controles sobre codigo/bench.cpp. Los lanza controles.sh.

Cada ronda baraja los puntos y mide cada uno en sus dos variantes, una detrás de la otra y en orden al azar,
para que la temperatura y el resto de la máquina afecten por igual a las dos.
"""
import argparse
import csv
import os
import random
import re
import subprocess
import sys
from pathlib import Path

IMPLS = ["Autores", "Libstdc++", "Williams", "Mutex"]
CONTROLES = {
    "alineacion": ([(1, 1), (10, 1), (10, 10)], [6, 12, 24]),
    "docker": ([(1, 1), (1, 10), (1, 50), (10, 1), (10, 10), (1000000, 50)], [1, 24]),
}
SEGUNDOS = 2


def medir(cmd, env):
    r = subprocess.run(cmd, env=env, capture_output=True, text=True, timeout=120)
    valores = re.findall(r"Throughput = (\S+) Mop/s", r.stdout)
    if r.returncode != 0 or not valores:
        print(f"  FALLO {cmd}: código {r.returncode}: {(r.stdout + r.stderr)[-300:]}", file=sys.stderr, flush=True)
        return ""
    return valores[0]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--control", choices=CONTROLES, required=True)
    ap.add_argument("--base", required=True, help="binario sin cambios, fuera de Docker")
    ap.add_argument("--otro", required=True, help="alineacion: binario alineado; docker: ruta dentro del contenedor")
    ap.add_argument("--jemalloc", required=True)
    ap.add_argument("--jemalloc-docker", default="/usr/lib/x86_64-linux-gnu/libjemalloc.so")
    ap.add_argument("--contenedor", default="")
    ap.add_argument("--docker", default="sudo docker")
    ap.add_argument("--salida", required=True)
    ap.add_argument("--rondas", type=int, default=3)
    a = ap.parse_args()

    env = dict(os.environ, LD_PRELOAD=a.jemalloc)
    if a.control == "docker":
        otro_prefijo, otro_env = a.docker.split() + ["exec", "-e", f"LD_PRELOAD={a.jemalloc_docker}", a.contenedor], dict(os.environ)
        nombres = ("fuera", "dentro")
    else:
        otro_prefijo, otro_env = [], env
        nombres = ("sin alinear", "alineado")

    paneles, hilos = CONTROLES[a.control]
    puntos = [(i, n, u, h) for i in IMPLS for (n, u) in paneles for h in hilos]
    salida = Path(a.salida)
    nuevo = not salida.exists()
    with salida.open("a", newline="") as f:
        w = csv.writer(f)
        if nuevo:
            w.writerow(["control", "variante", "impl", "n", "u", "hilos", "ronda", "mops"])
        for ronda in range(1, a.rondas + 1):
            random.shuffle(puntos)
            for k, (i, n, u, h) in enumerate(puntos, 1):
                argumentos = ["rendimiento", i, str(n), str(u), str(h), str(SEGUNDOS)]
                variantes = [(nombres[0], [a.base] + argumentos, env), (nombres[1], otro_prefijo + [a.otro] + argumentos, otro_env)]
                random.shuffle(variantes)
                for nombre, cmd, entorno in variantes:
                    w.writerow([a.control, nombre, i, n, u, h, ronda, medir(cmd, entorno)])
                    f.flush()
                if k % 12 == 0:
                    print(f"[{a.control}] ronda {ronda}: {k}/{len(puntos)}", flush=True)


if __name__ == "__main__":
    main()
