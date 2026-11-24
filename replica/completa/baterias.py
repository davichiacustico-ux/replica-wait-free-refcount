#!/usr/bin/env python3
"""Baterías de la réplica completa. Solo usa la biblioteca estándar, para correr igual dentro y fuera de Docker.

Motores (qué programa mide):
  oficial  binarios del artefacto de Zenodo: Autores, Libstdc++, Williams, Folly y Anderson
  equipo   codigo/bench.cpp del equipo vía bench_equipo: Autores, Libstdc++, Williams y Mutex

Experimentos:
  fig5       rendimiento, como la figura 5; el sistema operativo reparte los hilos
  fig6       latencia, como la figura 6
  afinidad   rendimiento con los hilos fijados a núcleos concretos («4 Xeon en miniatura»)
  latafin    latencia con esas mismas colocaciones
  basura     objetos muertos sin liberar (solo motor oficial)
  paridad    la receta exacta del paper (10 s x 7) en varios puntos, para validar los recortes de duración
  pares      2 hilos en distintos pares de núcleos: ¿un resultado depende de la distancia o de un par concreto?

Decisiones de método:
  - Cada ronda recorre todos los puntos en orden aleatorio, y cada punto es un proceso nuevo: si la CPU se
    calienta o alguien abre un programa, el efecto se reparte entre todas las implementaciones.
  - Los CSV se escriben línea a línea y al relanzar se saltan los puntos ya medidos.
"""
import argparse
import csv
import json
import os
import random
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path

CLUSTERS_E = [[8, 9, 10, 11], [12, 13, 14, 15], [16, 17, 18, 19], [20, 21, 22, 23]]   # comparten L2: los «sockets»
NUCLEOS_P = [2, 3, 4, 5, 6, 7, 0, 1]   # cada uno con su L2; el 0 y el 1 al final porque atienden más interrupciones

PANELES = [(1, 1), (1, 10), (1, 50), (10, 1), (10, 10), (1000000, 50)]
HILOS_FIG5 = [1, 3, 6, 9, 12, 18, 24, 30, 36]   # los del paper (1, 18, 36, ... 216) divididos entre 6
P_FIG6 = [1, 24, 56]                            # 1 y todos los núcleos; 56 es el valor literal del paper
PANELES_PARIDAD = PANELES
PANELES_PARES = [(1, 50)]
PARES = {"mismo clúster": [(8, 9), (12, 13), (16, 17), (20, 21)],
         "clústeres distintos": [(8, 12), (8, 16), (8, 20), (12, 16), (12, 20), (16, 20), (9, 13)],
         "P ↔ P": [(2, 3), (4, 5)], "P ↔ E": [(2, 12)]}

IMPL_OFICIAL = {"ivo": "Autores", "gnu": "Libstdc++", "jss-free": "Williams", "folly": "Folly", "mine": "Anderson"}
LAT_OFICIAL = {"Our": "Autores", "STL": "Libstdc++", "JSS-OS": "Williams", "Folly": "Folly", "Parlay": "Anderson"}
IMPL_EQUIPO = ["Autores", "Libstdc++", "Williams", "Mutex"]
OP_EQUIPO = {"load": "load", "store": "store", "store&reclaim": "store-del"}

CAMPOS_REND = ["experimento", "motor", "impl", "n", "u", "hilos", "colocacion", "familia", "ronda", "mops", "rss_kb",
               "segundos_reales", "carga1", "temp_c", "codigo"]
CAMPOS_LAT = ["experimento", "motor", "impl", "op", "p", "colocacion", "familia", "ronda", "p1", "p25", "p50", "p75",
              "p99", "p9995", "iteraciones", "carga1", "temp_c", "codigo"]


def colocaciones():
    """Devuelve (nombre, familia, hilos, cpus, cpu_del_hilo_principal)."""
    compacto = sum(CLUSTERS_E, [])
    disperso = [CLUSTERS_E[i % 4][i // 4] for i in range(16)]
    lista = [("E1", "base", 1, compacto[:1], 7)]
    for h in (2, 4, 8, 12):
        lista.append((f"E{h}-compacto", "E-compacto", h, compacto[:h], 7))
        lista.append((f"E{h}-disperso", "E-disperso", h, disperso[:h], 7))
    lista.append(("E16", "E-todos", 16, compacto, 7))
    for h in (1, 2, 4, 8):
        lista.append((f"P{h}", "P", h, NUCLEOS_P[:h], 23))
    lista.append(("todos24", "todos", 24, NUCLEOS_P + compacto, -1))
    return lista


def estado():
    """Carga y temperatura en el momento de medir, para poder detectar interferencias después."""
    carga = open("/proc/loadavg").read().split()[0]
    temps = []
    for z in Path("/sys/class/thermal").glob("thermal_zone*/temp"):
        try:
            temps.append(int(z.read_text()) / 1000)
        except (OSError, ValueError):
            pass
    return carga, f"{max(temps):.0f}" if temps else ""


def ejecutar(cmd, env, limite):
    """Corre un proceso y devuelve (código, salida, pico de memoria en KB, segundos)."""
    with tempfile.TemporaryFile("w+") as salida:
        t0 = time.time()
        p = subprocess.Popen(cmd, env=env, stdout=salida, stderr=subprocess.STDOUT)
        vigia = threading.Timer(limite, p.kill)
        vigia.start()
        _, estado_proc, uso = os.wait4(p.pid, 0)
        vigia.cancel()
        p.returncode = os.waitstatus_to_exitcode(estado_proc)
        salida.seek(0)
        return p.returncode, salida.read(), uso.ru_maxrss, time.time() - t0


class Registro:
    """CSV que se escribe línea a línea y recuerda qué puntos ya están medidos."""

    def __init__(self, ruta, campos, clave):
        self.ruta, self.campos, self.clave = ruta, campos, clave
        self.hechos = set()
        nuevo = not ruta.exists()
        if not nuevo:
            with open(ruta, newline="", encoding="utf-8") as f:
                self.hechos = {self._clave(r) for r in csv.DictReader(f)}
        self.f = open(ruta, "a", newline="", encoding="utf-8")
        self.w = csv.DictWriter(self.f, campos)
        if nuevo:
            self.w.writeheader()

    def _clave(self, fila):
        return tuple(str(fila[c]) for c in self.clave)

    def hecho(self, **fila):
        return self._clave(fila) in self.hechos

    def escribir(self, fila):
        self.w.writerow(fila)
        self.f.flush()
        self.hechos.add(self._clave(fila))


class Motor:
    def __init__(self, args):
        self.a = args
        self.nombre = args.motor
        raiz = Path(args.raiz)
        self.shim = str(Path(args.shim))
        self.numactl = ["numactl", "-i", "all"] if shutil.which("numactl") else []
        self.base = dict(os.environ)
        if args.biblios:
            self.base["LD_LIBRARY_PATH"] = args.biblios
        self.jemalloc = args.jemalloc
        if self.nombre == "oficial":
            self.pila = str(raiz / "concurrent_deferred_rc/build/benchmarks/bench_stack")
            self.lat = str(raiz / "atomic_shared_ptr/build/benchmark/bench_atomic_shared_ptrs")
            self.lat_williams = str(raiz / "atomic_shared_ptr/build_cx16/benchmark/bench_atomic_shared_ptrs")
            self.basura = str(raiz / "atomic_shared_ptr/build/benchmark/bench_basura")
            self.impls = list(IMPL_OFICIAL)
        else:
            self.equipo = args.equipo
            self.impls = IMPL_EQUIPO

    def entorno(self, cpus=None, principal=None, jemalloc=True, hilos=None):
        env = dict(self.base)
        precarga = []
        if jemalloc:
            precarga.append(self.jemalloc)
        if cpus is not None:
            precarga.append(self.shim)
            env["FIJAR_CPUS"] = ",".join(map(str, cpus))
            env["FIJAR_PRINCIPAL"] = str(principal)
            if self.a.registro_hilos:
                env["FIJAR_REGISTRO"] = self.a.registro_hilos
        if precarga:
            env["LD_PRELOAD"] = ":".join(precarga)
        if hilos is not None:
            env["NUM_THREADS"] = str(hilos)
        return env

    def rendimiento(self, impl, n, u, hilos, segundos, iteraciones, cpus=None, principal=None):
        """Devuelve (lista de Mop/s, rss, duración, código)."""
        env = self.entorno(cpus, principal, hilos=hilos)
        if self.nombre == "oficial":
            cmd = self.numactl + [self.pila, f"--threads={hilos}", f"--alg={impl}", "-r", str(segundos), "-s", str(n),
                                  "-u", str(u), "-i", str(iteraciones), "--peek=true"]
        else:
            cmd = self.numactl + [self.equipo, "rendimiento", impl, str(n), str(u), str(hilos), str(segundos)]
        cod, texto, rss, dur = ejecutar(cmd, env, self.a.limite + 12 * segundos * iteraciones)
        valores = [float(x) for x in re.findall(r"Throughput = (\S+) Mop/s", texto)]
        if cod != 0 or not valores:
            print(f"  FALLO {impl} n={n} u={u} h={hilos}: código {cod}: {texto[-300:]}", file=sys.stderr, flush=True)
        return valores, rss, dur, cod

    def latencia(self, impl, p, min_tiempo, cpus=None, principal=None):
        """Devuelve (filas con percentiles en ns, código)."""
        if self.nombre == "oficial":
            prefijo = next(k for k, v in LAT_OFICIAL.items() if v == impl)
            cmd = [self.lat_williams if impl == "Williams" else self.lat, f"--benchmark_filter=^{prefijo}::.*/{p}/manual_time",
                   "--benchmark_format=json", f"--benchmark_min_time={min_tiempo}s"]
            cod, texto, _, _ = ejecutar(cmd, self.entorno(cpus, principal, jemalloc=False), self.a.limite * 4)
            filas = []
            try:
                datos = json.loads(texto[texto.index("{"):])
            except ValueError:
                print(f"  FALLO latencia {impl} p={p}: código {cod}: {texto[-300:]}", file=sys.stderr, flush=True)
                return filas, cod
            for b in datos.get("benchmarks", []):
                op = b["name"].split("::")[1].split("/")[0]
                filas.append({"op": op, "iteraciones": b.get("iterations", ""),
                              **{c: f"{b[k] * 1e9:.1f}" for c, k in (("p1", "1%"), ("p25", "25%"), ("p50", "50%"),
                                                                     ("p75", "75%"), ("p99", "99%"), ("p9995", "99.95%"))}})
            return filas, cod
        cmd = [self.equipo, "latencia", impl, str(p), str(self.a.muestras)]
        cod, texto, _, _ = ejecutar(cmd, self.entorno(cpus, principal), self.a.limite * 4)
        filas = []
        for linea in texto.splitlines():
            c = linea.split(",")
            if len(c) == 8 and c[1] in OP_EQUIPO:
                filas.append({"op": OP_EQUIPO[c[1]], "iteraciones": self.a.muestras, "p1": c[3], "p25": c[4], "p50": c[5],
                              "p75": c[6], "p99": c[7], "p9995": ""})
        return filas, cod


def nombre_impl(motor, impl):
    return IMPL_OFICIAL.get(impl, impl) if motor.nombre == "oficial" else impl


def pasada_rendimiento(motor, reg, experimento, puntos, rondas, segundos):
    total = len(puntos) * rondas
    hecho = 0
    t0 = time.time()
    for ronda in range(1, rondas + 1):
        orden = list(puntos)
        random.Random(1000 * ronda + len(puntos)).shuffle(orden)
        for impl, n, u, hilos, col, fam, cpus, principal in orden:
            hecho += 1
            base = dict(experimento=experimento, motor=motor.nombre, impl=nombre_impl(motor, impl), n=n, u=u, hilos=hilos,
                        colocacion=col, ronda=ronda)
            if reg.hecho(**base):
                continue
            carga, temp = estado()
            valores, rss, dur, cod = motor.rendimiento(impl, n, u, hilos, segundos, 1, cpus, principal)
            reg.escribir({**base, "familia": fam, "mops": valores[0] if valores else "", "rss_kb": rss,
                          "segundos_reales": f"{dur:.2f}", "carga1": carga, "temp_c": temp, "codigo": cod})
            if hecho % 50 == 0:
                print(f"[{experimento}] {hecho}/{total}  {(time.time() - t0) / 60:.1f} min  carga {carga}  {temp} °C", flush=True)


def pasada_latencia(motor, reg, experimento, puntos, rondas, min_tiempo):
    total = len(puntos) * rondas
    hecho = 0
    t0 = time.time()
    for ronda in range(1, rondas + 1):
        orden = list(puntos)
        random.Random(2000 * ronda + len(puntos)).shuffle(orden)
        for impl, p, col, fam, cpus, principal in orden:
            hecho += 1
            base = dict(experimento=experimento, motor=motor.nombre, impl=impl, p=p, colocacion=col, ronda=ronda)
            if reg.hecho(**base, op="load"):
                continue
            carga, temp = estado()
            filas, cod = motor.latencia(impl, p, min_tiempo, cpus, principal)
            for f in filas:
                reg.escribir({**base, "familia": fam, "carga1": carga, "temp_c": temp, "codigo": cod, **f})
            if not filas:
                print(f"  sin datos: {impl} p={p} {col} código {cod}", file=sys.stderr, flush=True)
            if hecho % 25 == 0:
                print(f"[{experimento}] {hecho}/{total}  {(time.time() - t0) / 60:.1f} min  carga {carga}  {temp} °C", flush=True)


def impls_lat(motor):
    return list(LAT_OFICIAL.values()) if motor.nombre == "oficial" else IMPL_EQUIPO


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--motor", choices=["oficial", "equipo"], required=True)
    ap.add_argument("--salida", required=True)
    ap.add_argument("--raiz", default="/trabajo", help="carpeta del artefacto compilado")
    ap.add_argument("--shim", default="/trabajo/fijar_hilos.so")
    ap.add_argument("--jemalloc", default="/usr/lib/x86_64-linux-gnu/libjemalloc.so")
    ap.add_argument("--biblios", default="", help="LD_LIBRARY_PATH para las dependencias locales")
    ap.add_argument("--equipo", default="", help="ruta de bench_equipo")
    ap.add_argument("--solo", default="fig5,fig6,afinidad,latafin,pares,basura,paridad")
    ap.add_argument("--rondas-fig5", type=int, default=5)
    ap.add_argument("--rondas-afinidad", type=int, default=3)
    ap.add_argument("--segundos", type=float, default=2.0)
    ap.add_argument("--min-tiempo-fig6", type=float, default=0.5, help="el valor por omisión de Google Benchmark")
    ap.add_argument("--min-tiempo-afinidad", type=float, default=0.2)
    ap.add_argument("--muestras", type=int, default=100000)
    ap.add_argument("--limite", type=float, default=120.0)
    ap.add_argument("--registro-hilos", default="")
    a = ap.parse_args()

    salida = Path(a.salida)
    salida.mkdir(parents=True, exist_ok=True)
    motor = Motor(a)
    solo = set(a.solo.split(","))
    rend = Registro(salida / "rendimiento.csv", CAMPOS_REND, ["experimento", "impl", "n", "u", "hilos", "colocacion", "ronda"])
    lat = Registro(salida / "latencia.csv", CAMPOS_LAT, ["experimento", "impl", "op", "p", "colocacion", "ronda"])

    if "fig5" in solo:
        puntos = [(i, n, u, h, "sistema", "sistema", None, None) for (n, u) in PANELES for h in HILOS_FIG5 for i in motor.impls]
        pasada_rendimiento(motor, rend, "fig5", puntos, a.rondas_fig5, a.segundos)
    if "fig6" in solo:
        puntos = [(i, p, "sistema", "sistema", None, None) for p in P_FIG6 for i in impls_lat(motor)]
        pasada_latencia(motor, lat, "fig6", puntos, a.rondas_fig5, a.min_tiempo_fig6)
    if "afinidad" in solo:
        puntos = [(i, n, u, h, col, fam, cpus, pr) for (col, fam, h, cpus, pr) in colocaciones()
                  for (n, u) in PANELES for i in motor.impls]
        pasada_rendimiento(motor, rend, "afinidad", puntos, a.rondas_afinidad, a.segundos)
    if "latafin" in solo:
        # el hilo medido debe caer en la lista: FIJAR_PRINCIPAL=lista lo trata como el primer hilo
        puntos = [(i, h, col, fam, cpus, "lista") for (col, fam, h, cpus, _) in colocaciones() if h > 1 for i in impls_lat(motor)]
        pasada_latencia(motor, lat, "latafin", puntos, a.rondas_afinidad, a.min_tiempo_afinidad)
    if "pares" in solo:
        puntos = [(i, n, u, 2, f"par-{x}-{y}", clase, [x, y], 23 if x < 8 else 7)
                  for clase, pares in PARES.items() for (x, y) in pares for (n, u) in PANELES_PARES for i in motor.impls]
        pasada_rendimiento(motor, rend, "pares", puntos, a.rondas_afinidad, a.segundos)
    if "basura" in solo and motor.nombre == "oficial":
        ruta = salida / "basura.csv"
        with open(ruta, "w", encoding="utf-8") as f:
            for ronda in range(1, a.rondas_afinidad + 1):
                cod, texto, _, _ = ejecutar([motor.basura, str(a.segundos), "2", "4", "8", "16", "24"], motor.entorno(jemalloc=False), 600)
                lineas = texto.strip().splitlines()
                if ronda == 1:
                    f.write("ronda," + lineas[0] + "\n")
                f.writelines(f"{ronda},{l}\n" for l in lineas[1:])
                f.flush()
                print(f"[basura] ronda {ronda} código {cod}", flush=True)
    if "paridad" in solo and motor.nombre == "oficial":
        ruta = salida / "paridad.csv"
        with open(ruta, "w", encoding="utf-8") as f:
            f.write("impl,n,u,hilos,mediana_7x10s,minimo,maximo\n")
            for (n, u) in PANELES_PARIDAD:
                for i in motor.impls:
                    valores, _, _, cod = motor.rendimiento(i, n, u, 24, 10, 7)
                    v = sorted(valores)
                    if v:
                        f.write(f"{IMPL_OFICIAL[i]},{n},{u},24,{v[len(v) // 2]},{v[0]},{v[-1]}\n")
                        f.flush()
                    print(f"[paridad] {i} n={n} u={u}: {v}", flush=True)
    print("BATERIAS TERMINADAS", flush=True)


if __name__ == "__main__":
    main()
