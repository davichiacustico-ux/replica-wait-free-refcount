#!/usr/bin/env python3
"""¿El escenario n = 10⁶ se queda sin RAM? Cuenta los bytes reales que pasan por la controladora de memoria.

Usa los contadores uncore_imc_free_running de la CPU (herramientas/contar_imc.c), así que no es una estimación:
es lo que entró y salió de la RAM.
Mide tres cosas y escribe <resultados>/memoria_ram.csv:
  reposo         lo que mueve la máquina sin hacer nada, para saber cuánto ruido hay de fondo
  lectura/copia  el techo real en orden, con herramientas/ancho_banda.cpp
  azar           el techo con líneas sueltas, que es como acceden las pilas
  pila           cada algoritmo con n = 10⁶ y u = 50 %, con el binario oficial dentro de Docker

Necesita sudo sin contraseña, la imagen replica-wait-free y artefacto_docker/ compilado.
Con la máquina libre. Tarda unos 20 minutos.
"""
import csv
import random
import re
import subprocess
from pathlib import Path

AQUI = Path(__file__).resolve().parent.parent
SALIDA = AQUI.parent / "resultados" / "david-ultra9-285" / "memoria_ram.csv"
IMAGEN = "replica-wait-free"
JEMALLOC = "/usr/lib/x86_64-linux-gnu/libjemalloc.so"
IMPLS = {"ivo": "Autores", "gnu": "Libstdc++", "jss-free": "Williams", "folly": "Folly", "mine": "Anderson"}
HILOS_PILA = [1, 6, 12, 18, 24]
HILOS_TECHO = [1, 8, 24]
RONDAS = 2
SEGUNDOS_PILA = 14
SEGUNDOS_TECHO = 12
CONTADOR = AQUI / "bin" / "contar_imc"
CAMPOS = ["prueba", "impl", "hilos", "ronda", "mops", "gbs_lectura", "gbs_escritura", "bytes_por_op"]


def con_contadores(cmd):
    """Ejecuta cmd midiendo la RAM. Devuelve su salida y los GB/s leídos y escritos en régimen estable."""
    r = subprocess.run(["sudo", "-n", str(CONTADOR), "500"] + cmd, capture_output=True, text=True, timeout=300)
    tramos = [[float(x) for x in l.split(",")[1:]] for l in r.stderr.splitlines() if l.startswith("IMC,")]
    # El contador avanza a saltos de 1 GB: la mediana de cada tramo lo redondearía, así que se usa el total de una ventana larga.
    # Rellenar y liberar las pilas lleva hasta 4 s, así que solo vale la mitad central de la ejecución.
    cuarto = len(tramos) // 4
    ventana = tramos[cuarto:-cuarto]
    dur = ventana[-1][0] - tramos[cuarto - 1][0]
    return r.stdout, sum(t[1] for t in ventana) / dur, sum(t[2] for t in ventana) / dur


def pila(alg, hilos):
    return ["docker", "run", "--rm", "--cap-add", "SYS_NICE", "--mount", f"type=bind,src={AQUI}/artefacto_docker,dst=/trabajo,readonly",
            "-e", f"LD_PRELOAD={JEMALLOC}", "-e", f"NUM_THREADS={hilos}", IMAGEN,
            "/trabajo/concurrent_deferred_rc/build/benchmarks/bench_stack", f"--threads={hilos}", f"--alg={alg}",
            "-r", str(SEGUNDOS_PILA), "-s", "1000000", "-u", "50", "-i", "1", "--peek=true"]


def main():
    binario = AQUI / "bin" / "ancho_banda"
    subprocess.run(["g++-12", "-std=c++20", "-O3", "-march=native", "-pthread", str(AQUI / "herramientas" / "ancho_banda.cpp"),
                    "-o", str(binario)], check=True)
    subprocess.run(["gcc", "-O2", str(AQUI / "herramientas" / "contar_imc.c"), "-o", str(CONTADOR)], check=True)
    with open(SALIDA, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, CAMPOS)
        w.writeheader()

        def anotar(prueba, impl, hilos, ronda, mops, leido, escrito):
            bytes_op = f"{1000 * (leido + escrito) / mops:.0f}" if mops else ""
            w.writerow(dict(prueba=prueba, impl=impl, hilos=hilos, ronda=ronda, mops=mops or "", gbs_lectura=f"{leido:.2f}",
                            gbs_escritura=f"{escrito:.2f}", bytes_por_op=bytes_op))
            f.flush()
            print(prueba, impl, hilos, ronda, mops, f"{leido:.2f} {escrito:.2f} GB/s", bytes_op, flush=True)

        for ronda in range(1, RONDAS + 1):
            _, leido, escrito = con_contadores(["sleep", str(SEGUNDOS_TECHO)])
            anotar("reposo", "", 0, ronda, None, leido, escrito)
            for modo in ("lectura", "copia", "azar"):
                for hilos in HILOS_TECHO:
                    _, leido, escrito = con_contadores([str(binario), modo, str(hilos), str(SEGUNDOS_TECHO)])
                    anotar(modo, "", hilos, ronda, None, leido, escrito)
            puntos = [(alg, h) for alg in IMPLS for h in HILOS_PILA]
            random.Random(ronda).shuffle(puntos)
            for alg, hilos in puntos:
                salida, leido, escrito = con_contadores(pila(alg, hilos))
                m = re.search(r"Throughput = (\S+) Mop/s", salida)
                anotar("pila", IMPLS[alg], hilos, ronda, float(m.group(1)) if m else None, leido, escrito)
    print("MEMORIA TERMINADA", flush=True)


if __name__ == "__main__":
    main()
