#!/usr/bin/env python3
"""Tablas y gráficas de la réplica completa.

Uso: python3 replica/completa/analizar.py [carpeta_de_resultados]
Lee las carpetas docker, nativo, nativo_oficial y nativo_alineado de <carpeta>, y escribe <carpeta>/figuras/*.png y <carpeta>/tablas.md.

Paleta: misma asignación de tonos que la figura 5 del paper, con pasos ajustados porque la original no
distingue Williams de Folly con daltonismo (protanopia, ΔE 0,7). Comprobada con validate_palette.js: ΔE ≥ 16.
Mutex no es rival del paper: va en gris, como línea de referencia.
"""
import csv
import statistics as st
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap, TwoSlopeNorm

AQUI = Path(__file__).resolve().parent
RES = Path(sys.argv[1]) if len(sys.argv) > 1 else AQUI.parent / "resultados" / "david-ultra9-285"
FIG = RES / "figuras"
FIG.mkdir(exist_ok=True)

COL = {"Autores": "#c8232c", "Libstdc++": "#1f6fb4", "Folly": "#e39b1e", "Williams": "#0f7f52", "Anderson": "#8b5cc4", "Mutex": "#6f7278"}
MARCA = {"Autores": "o", "Libstdc++": "s", "Folly": "P", "Williams": "X", "Anderson": "*", "Mutex": "^"}
ORDEN = ["Autores", "Anderson", "Folly", "Williams", "Libstdc++"]
TINTA, TINTA2, REJILLA, FONDO = "#1f1f1e", "#6b6b67", "#e4e3de", "#fcfcfb"
PANELES = [("1", "1"), ("1", "10"), ("1", "50"), ("10", "1"), ("10", "10"), ("1000000", "50")]
OPS = [("load", "load"), ("store", "store"), ("store-del", "store & reclaim")]
HILOS_PAPER_TODOS, HILOS_TODOS = 144, 24
# medianas impresas en la figura 6 del paper, en ns (leídas de la figura ampliada)
PAPER_FIG6 = {
    ("load", 1): {"Autores": 23, "Libstdc++": 29, "Folly": 24, "Anderson": 28},
    ("store", 1): {"Autores": 33, "Libstdc++": 36, "Folly": 37, "Anderson": 28},
    ("store-del", 1): {"Autores": 44, "Libstdc++": 41, "Folly": 49, "Anderson": 44},
    ("load", 56): {"Autores": 1140, "Libstdc++": 99700, "Folly": 11600, "Anderson": 2580},
    ("store", 56): {"Autores": 1470, "Libstdc++": 111000, "Folly": 2620, "Anderson": 2840},
    ("store-del", 56): {"Autores": 1570, "Libstdc++": 59300, "Folly": 5060, "Anderson": 1250},
}

plt.rcParams.update({
    "font.size": 10, "font.family": "DejaVu Sans", "text.color": TINTA, "axes.labelcolor": TINTA2, "axes.edgecolor": REJILLA,
    "xtick.color": TINTA2, "ytick.color": TINTA2, "axes.spines.top": False, "axes.spines.right": False, "axes.grid": True,
    "grid.color": REJILLA, "grid.linewidth": 0.8, "axes.axisbelow": True, "figure.facecolor": FONDO, "axes.facecolor": FONDO,
    "savefig.facecolor": FONDO, "legend.frameon": False, "axes.titlesize": 11, "axes.titleweight": "bold", "lines.linewidth": 2,
})


def titulo_panel(n, u):
    return f"n = {'10⁶' if n == '1000000' else n}, u = {u} %"


def leer(ruta):
    if not ruta.exists():
        return []
    with open(ruta, newline="", encoding="utf-8") as f:
        return [r for r in csv.DictReader(f)]


def serie(filas, **filtro):
    return [r for r in filas if all(r[k] == str(v) for k, v in filtro.items())]


def valores(filas, campo="mops", **filtro):
    return [float(r[campo]) for r in serie(filas, **filtro) if r[campo] not in ("", None)]


def mediana(filas, campo="mops", **filtro):
    v = valores(filas, campo, **filtro)
    return st.median(v) if v else None


def guardar(fig, nombre):
    fig.savefig(FIG / nombre, dpi=160, bbox_inches="tight")
    plt.close(fig)
    print("  ", nombre)


def leyenda_superior(fig, impls, y=1.02):
    manijas = [plt.Line2D([], [], color=COL[i], marker=MARCA[i], ms=7, label=i) for i in impls]
    fig.legend(handles=manijas, loc="lower center", bbox_to_anchor=(0.5, y), ncol=len(impls), handlelength=2.2, columnspacing=1.8)


def figura_rendimiento(filas, impls, nombre, experimento="fig5"):
    fig, ejes = plt.subplots(2, 3, figsize=(11.5, 6), constrained_layout=True)
    for ax, (n, u) in zip(ejes.flat, PANELES):
        for impl in impls:
            hilos = sorted({int(r["hilos"]) for r in serie(filas, experimento=experimento, impl=impl, n=n, u=u)})
            med, bajo, alto = [], [], []
            for h in hilos:
                v = valores(filas, experimento=experimento, impl=impl, n=n, u=u, hilos=h)
                med.append(st.median(v)), bajo.append(min(v)), alto.append(max(v))
            if not hilos:
                continue
            ax.fill_between(hilos, bajo, alto, color=COL[impl], alpha=0.16, lw=0)
            ax.plot(hilos, med, color=COL[impl], marker=MARCA[impl], ms=6, mec=FONDO, mew=0.6)
        ax.axvline(HILOS_TODOS, color=TINTA2, ls="--", lw=1)
        ax.set_title(titulo_panel(n, u))
        ax.set_xticks([1, 6, 12, 18, 24, 30, 36])
        ax.set_ylim(bottom=0)
    ejes[0, 0].text(HILOS_TODOS + 0.6, 0.97, "todos los núcleos", transform=ejes[0, 0].get_xaxis_transform(), fontsize=7.5, color=TINTA2, va="top")
    for ax in ejes[1]:
        ax.set_xlabel("número de hilos")
    for ax in ejes[:, 0]:
        ax.set_ylabel("rendimiento (Mop/s)")
    leyenda_superior(fig, impls)
    guardar(fig, nombre)


def kendall(a, b):
    claves = [k for k in a if k in b and a[k] is not None and b[k] is not None]
    acuerdo = total = 0
    for i, x in enumerate(claves):
        for y in claves[i + 1:]:
            total += 1
            acuerdo += 1 if (a[x] - a[y]) * (b[x] - b[y]) > 0 else -1
    return acuerdo / total if total else None


def tabla_paper_replica(filas, paper, salida):
    salida.append("\n## Rendimiento: paper (144 hilos) frente a réplica (24 hilos)\n")
    salida.append("Mop/s y, entre paréntesis, relación con Autores. τ = acuerdo de orden entre paper y réplica (1 = idéntico).\n")
    salida.append("| Panel | | " + " | ".join(ORDEN) + " | τ |")
    salida.append("|---|---|" + "---|" * (len(ORDEN) + 1))
    for n, u in PANELES:
        p = {i: mediana(paper, impl=i, n=n, u=u, hilos=HILOS_PAPER_TODOS) for i in ORDEN}
        r = {i: mediana(filas, experimento="fig5", impl=i, n=n, u=u, hilos=HILOS_TODOS) for i in ORDEN}
        tau = kendall(p, r)
        for rotulo, d in (("paper", p), ("réplica", r)):
            base = d.get("Autores")
            celdas = [f"{d[i]:.2f} ({d[i] / base:.2f}×)" if d.get(i) is not None and base else "—" for i in ORDEN]
            fin = f"{tau:+.2f}" if tau is not None and rotulo == "réplica" else ""
            salida.append(f"| {titulo_panel(n, u) if rotulo == 'paper' else ''} | {rotulo} | " + " | ".join(celdas) + f" | {fin} |")


def figura_comparacion(filas, paper):
    """Cuánto rinde cada rival respecto a Autores, en el paper y en la réplica."""
    fig, ejes = plt.subplots(2, 3, figsize=(11.5, 5.4), constrained_layout=True, sharex=True, sharey=True)
    rivales = [i for i in ORDEN if i != "Autores"]
    for ax, (n, u) in zip(ejes.flat, PANELES):
        pa = mediana(paper, impl="Autores", n=n, u=u, hilos=HILOS_PAPER_TODOS)
        ra = mediana(filas, experimento="fig5", impl="Autores", n=n, u=u, hilos=HILOS_TODOS)
        for y, impl in enumerate(rivales):
            p = mediana(paper, impl=impl, n=n, u=u, hilos=HILOS_PAPER_TODOS)
            r = mediana(filas, experimento="fig5", impl=impl, n=n, u=u, hilos=HILOS_TODOS)
            if not (p and r and pa and ra):
                continue
            ax.plot([p / pa, r / ra], [y, y], color=COL[impl], lw=2, alpha=0.45)
            ax.plot(p / pa, y, marker="o", ms=8, mfc=FONDO, mec=COL[impl], mew=2)
            ax.plot(r / ra, y, marker="o", ms=8, color=COL[impl], mec=FONDO, mew=0.6)
        ax.axvline(1, color=TINTA2, lw=1)
        ax.set_xscale("log")
        ax.set_yticks(range(len(rivales)), rivales)
        ax.invert_yaxis()
        ax.set_title(titulo_panel(n, u))
        ax.grid(axis="y", visible=False)
    fig.supxlabel("rendimiento relativo a Autores (1 = igual; a la derecha, gana el rival)", color=TINTA2, fontsize=10)
    manijas = [plt.Line2D([], [], ls="", marker="o", ms=8, mfc=FONDO, mec=TINTA2, mew=2, label="paper (144 hilos)"),
               plt.Line2D([], [], ls="", marker="o", ms=8, color=TINTA2, label="réplica (24 hilos)")]
    fig.legend(handles=manijas, loc="lower center", bbox_to_anchor=(0.5, 1.0), ncol=2)
    guardar(fig, "fig5_paper_frente_a_replica.png")


def tabla_un_hilo(filas, paper, salida):
    """Con un hilo no hay contención ni líneas viajando entre sockets: lo que difiera aquí no viene de ahí."""
    rivales = [i for i in ORDEN if i != "Autores"]
    salida.append("\n## Relación con Autores con UN solo hilo\n")
    salida.append("Rendimiento del rival dividido entre el de Autores.\n")
    salida.append("| Panel | | " + " | ".join(rivales) + " |")
    salida.append("|---|---|" + "---|" * len(rivales))
    for n, u in PANELES:
        for rotulo, datos, extra in (("paper", paper, {}), ("réplica", filas, {"experimento": "fig5"})):
            base = mediana(datos, impl="Autores", n=n, u=u, hilos=1, **extra)
            celdas = [f"{mediana(datos, impl=i, n=n, u=u, hilos=1, **extra) / base:.2f}×" for i in rivales]
            salida.append(f"| {titulo_panel(n, u) if rotulo == 'paper' else ''} | {rotulo} | " + " | ".join(celdas) + " |")


def percentiles(filas, **filtro):
    claves = ("p1", "p25", "p50", "p75", "p99")
    v = [st.median(valores(filas, k, **filtro) or [float("nan")]) for k in claves]
    return dict(zip(claves, v))


def figura_latencia(filas, impls, p_alto, nombre):
    fig, ejes = plt.subplots(2, 3, figsize=(11.5, 6), constrained_layout=True)
    for fila_ejes, p, escala in ((ejes[0], 1, "linear"), (ejes[1], p_alto, "log")):
        for ax, (op, rotulo) in zip(fila_ejes, OPS):
            cajas, etiquetas = [], []
            for impl in impls:
                q = percentiles(filas, experimento="fig6", impl=impl, op=op, p=p)
                if q["p50"] == q["p50"]:
                    cajas.append({"label": impl, "whislo": q["p1"], "q1": q["p25"], "med": q["p50"], "q3": q["p75"], "whishi": q["p99"], "fliers": []})
                    etiquetas.append(impl)
            if not cajas:
                continue
            b = ax.bxp(cajas, showfliers=False, patch_artist=True, widths=0.38, medianprops={"color": TINTA, "lw": 1.6})
            for caja, impl in zip(b["boxes"], etiquetas):
                caja.set_facecolor(COL[impl] + "33")
                caja.set_edgecolor(COL[impl])
            for tipo in ("whiskers", "caps"):
                for linea, impl in zip(b[tipo], [i for i in etiquetas for _ in (0, 1)]):
                    linea.set_color(COL[impl])
            for x, c in enumerate(cajas, 1):
                m = c["med"]
                ax.annotate(f"{m:.0f} ns" if m < 1000 else f"{m / 1000:.2f} µs".replace(".", ","), (x + 0.21, m), xytext=(1, 0),
                            textcoords="offset points", ha="left", va="center", fontsize=7.5, color=TINTA)
            ax.set_yscale(escala)
            ax.set_xlim(0.5, len(cajas) + 0.85)
            if escala == "linear":
                ax.set_ylim(bottom=0)
            ax.set_title(f"{rotulo} (p = {p})")
            ax.tick_params(axis="x", labelsize=8.5)
            ax.grid(axis="x", visible=False)
    ejes[0, 0].set_ylabel("latencia (ns)")
    ejes[1, 0].set_ylabel("latencia (ns, escala log.)")
    guardar(fig, nombre)


def tabla_latencia(filas, salida, nombre="figura 6"):
    salida.append(f"\n## Latencia: medianas en ns ({nombre})\n")
    salida.append("| Operación | p | " + " | ".join(ORDEN) + " |")
    salida.append("|---|---|" + "---|" * len(ORDEN))
    for op, rotulo in OPS:
        for p_paper, p_aqui in ((1, 1), (56, 24), (56, 56)):
            pap = PAPER_FIG6[(op, p_paper)]
            salida.append(f"| {rotulo} | paper p={p_paper} | " + " | ".join(f"{pap[i]:,}" if i in pap else "—" for i in ORDEN) + " |")
            r = {i: mediana(filas, "p50", experimento="fig6", impl=i, op=op, p=p_aqui) for i in ORDEN}
            salida.append(f"| | réplica p={p_aqui} | " + " | ".join(f"{r[i]:,.0f}" if r[i] else "—" for i in ORDEN) + " |")


def tabla_colas(filas, salida, sufijo=""):
    salida.append(f"\n## Colas de latencia en ns: p50 / p99 / p99,95{sufijo}\n")
    salida.append("| Operación | p | " + " | ".join(ORDEN) + " |")
    salida.append("|---|---|" + "---|" * len(ORDEN))
    for op, rotulo in OPS:
        for p in (1, 24):
            celdas = []
            for i in ORDEN:
                q = [mediana(filas, c, experimento="fig6", impl=i, op=op, p=p) for c in ("p50", "p99", "p9995")]
                celdas.append(" / ".join(f"{v:,.0f}" for v in q) if all(q) else "—")
            salida.append(f"| {rotulo} | {p} | " + " | ".join(celdas) + " |")


def figura_topologia():
    filas = leer(RES / "topologia" / "matriz_nucleos.csv")
    if not filas:
        return
    m = [[float("nan")] * 24 for _ in range(24)]
    for r in filas:
        m[int(r["a"])][int(r["b"])] = float(r["ns"])
    tonos = LinearSegmentedColormap.from_list("azul", ["#e8f1fc", "#9ec5f4", "#3987e5", "#184f95", "#0d366b"])
    fig, ax = plt.subplots(figsize=(7.6, 6.6), constrained_layout=True)
    im = ax.imshow(m, cmap=tonos, vmin=30, vmax=95)
    ax.grid(False)
    for corte in (7.5, 11.5, 15.5, 19.5):
        ax.axhline(corte, color=FONDO, lw=2)
        ax.axvline(corte, color=FONDO, lw=2)
    ax.set_xticks(range(0, 24, 2))
    ax.set_yticks(range(0, 24, 2))
    ax.set_xlabel("núcleo que responde")
    ax.set_ylabel("núcleo que lanza")
    for x, texto in ((3.5, "8 núcleos P"), (9.5, "E «socket» 0"), (13.5, "E «socket» 1"), (17.5, "E «socket» 2"), (21.5, "E «socket» 3")):
        ax.text(x, -1.2, texto, ha="center", fontsize=8, color=TINTA)
    fig.colorbar(im, ax=ax, label="latencia de un viaje de línea de caché (ns)", shrink=0.8)
    guardar(fig, "topologia.png")


def clases_topologia():
    filas = leer(RES / "topologia" / "matriz_nucleos.csv")
    grupo = {c: ("P", c) for c in range(8)}
    for i, base in enumerate((8, 12, 16, 20)):
        grupo.update({c: ("E", i) for c in range(base, base + 4)})
    clases = defaultdict(list)
    for r in filas:
        a, b = grupo[int(r["a"])], grupo[int(r["b"])]
        if a[0] == b[0] == "P":
            k = "P ↔ P"
        elif a[0] != b[0]:
            k = "P ↔ E"
        else:
            k = "E, mismo clúster" if a[1] == b[1] else "E, clústeres distintos"
        clases[k].append(float(r["ns"]))
    return {k: st.median(v) for k, v in clases.items()}


def figura_dosis(filas, impls, nombre):
    """Mismos 4 hilos; lo único que cambia es la distancia entre ellos."""
    cl = clases_topologia()
    x = ["E4-compacto", "E4-disperso", "P4"]
    rot = [f"mismo clúster\n{cl.get('E, mismo clúster', 0):.0f} ns", f"4 clústeres\n{cl.get('E, clústeres distintos', 0):.0f} ns",
           f"4 núcleos P\n{cl.get('P ↔ P', 0):.0f} ns"]
    fig, ejes = plt.subplots(2, 3, figsize=(11.5, 6), constrained_layout=True)
    for ax, (n, u) in zip(ejes.flat, PANELES):
        for impl in impls:
            y = [mediana(filas, experimento="afinidad", impl=impl, n=n, u=u, colocacion=c) for c in x]
            if all(v is not None for v in y):
                ax.plot(range(3), y, color=COL[impl], marker=MARCA[impl], ms=7, mec=FONDO, mew=0.6)
        ax.set_xticks(range(3), rot, fontsize=8.5)
        ax.set_xlim(-0.25, 2.25)
        ax.set_ylim(bottom=0)
        ax.set_title(titulo_panel(n, u))
        ax.grid(axis="x", visible=False)
    for ax in ejes[:, 0]:
        ax.set_ylabel("rendimiento (Mop/s), 4 hilos")
    leyenda_superior(fig, impls)
    guardar(fig, nombre)


def tabla_compacto_disperso(filas, impls, salida, nombre):
    """Cambio porcentual al pasar de un clúster a varios, con los mismos núcleos tipo E."""
    salida.append(f"\n## Efecto de separar los hilos entre «sockets» ({nombre})\n")
    salida.append("Cambio de rendimiento de `E-compacto` a `E-disperso`, en %. «≈» = los rangos de las rondas se solapan: no se distingue del ruido.\n")
    for h in (2, 4, 8, 12):
        salida.append(f"\n**{h} hilos**\n")
        salida.append("| Panel | " + " | ".join(impls) + " |")
        salida.append("|---|" + "---|" * len(impls))
        for n, u in PANELES:
            celdas = []
            for impl in impls:
                c = valores(filas, experimento="afinidad", impl=impl, n=n, u=u, colocacion=f"E{h}-compacto")
                d = valores(filas, experimento="afinidad", impl=impl, n=n, u=u, colocacion=f"E{h}-disperso")
                if not c or not d:
                    celdas.append("—")
                    continue
                cambio = 100 * (st.median(d) / st.median(c) - 1)
                solapa = max(min(c), min(d)) <= min(max(c), max(d))
                celdas.append(f"{'≈' if solapa else ''}{cambio:+.0f} %")
            salida.append(f"| {titulo_panel(n, u)} | " + " | ".join(celdas) + " |")


def tabla_pares(filas, impls, salida, nombre):
    """Mop/s con 2 hilos, n = 1, u = 50 %, en distintos pares de núcleos."""
    pares = {r["colocacion"]: r["familia"] for r in serie(filas, experimento="pares")}
    if not pares:
        return
    rondas = len({r["ronda"] for r in serie(filas, experimento="pares")})
    salida.append(f"\n## Dos hilos en distintos pares de núcleos ({nombre})\n")
    salida.append(f"Mop/s, mediana de {rondas} rondas (entre paréntesis, mínimo y máximo de Williams), n = 1, u = 50 %.\n")
    salida.append("| Clase | Núcleos | " + " | ".join(impls) + " |")
    salida.append("|---|---|" + "---|" * len(impls))
    clases = ("mismo clúster", "clústeres distintos", "P ↔ P", "P ↔ E")
    for par, clase in sorted(pares.items(), key=lambda kv: (clases.index(kv[1]), kv[0])):
        celdas = []
        for impl in impls:
            v = valores(filas, experimento="pares", impl=impl, colocacion=par)
            celdas.append(f"{st.median(v):.2f}" + (f" ({min(v):.2f}–{max(v):.2f})" if impl == "Williams" else "") if v else "—")
        salida.append(f"| {clase} | {par[4:].replace('-', ' y ')} | " + " | ".join(celdas) + " |")


def figura_mapa_distancia(filas, impls, h, nombre):
    """Mapa impl × panel del cambio al dispersar: azul = pierde al separar, naranja = gana."""
    m = [[None] * len(PANELES) for _ in impls]
    for i, impl in enumerate(impls):
        for j, (n, u) in enumerate(PANELES):
            c = mediana(filas, experimento="afinidad", impl=impl, n=n, u=u, colocacion=f"E{h}-compacto")
            d = mediana(filas, experimento="afinidad", impl=impl, n=n, u=u, colocacion=f"E{h}-disperso")
            m[i][j] = 100 * (d / c - 1) if c and d else float("nan")
    tonos = LinearSegmentedColormap.from_list("div", ["#184f95", "#6da7ec", "#d9d9d6", "#f0a57c", "#a94313"])
    fig, ax = plt.subplots(figsize=(8.6, 3.5), constrained_layout=True)
    im = ax.imshow(m, cmap=tonos, norm=TwoSlopeNorm(vcenter=0, vmin=-60, vmax=60), aspect="auto")
    ax.grid(False)
    ax.set_xticks(range(len(PANELES)), [titulo_panel(*p) for p in PANELES], fontsize=8.5)
    ax.set_yticks(range(len(impls)), impls)
    for i in range(len(impls)):
        for j in range(len(PANELES)):
            v = m[i][j]
            if v == v:
                ax.text(j, i, f"{v:+.0f} %", ha="center", va="center", fontsize=9, color="#ffffff" if abs(v) > 42 else TINTA)
    fig.colorbar(im, ax=ax, label="cambio al separar los hilos (%)", shrink=0.9)
    guardar(fig, nombre)


def figura_miniatura(filas, paper):
    """Mismo porcentaje de máquina ocupada: 4 Xeon reales frente a 4 clústeres E."""
    paneles = [("1", "1"), ("10", "1"), ("1000000", "50")]
    fig, ejes = plt.subplots(2, 3, figsize=(11.5, 6), constrained_layout=True)
    for columna, (n, u) in enumerate(paneles):
        ax = ejes[0, columna]
        for impl in ORDEN:
            hs = [1, 18, 36, 54, 72]
            y = [mediana(paper, impl=impl, n=n, u=u, hilos=h) for h in hs]
            if all(v is not None for v in y):
                ax.plot([100 * h / 72 for h in hs], y, color=COL[impl], marker=MARCA[impl], ms=6, mec=FONDO, mew=0.6)
        ax.set_title(f"Paper · 4 Xeon · {titulo_panel(n, u)}", fontsize=10)
        ax = ejes[1, columna]
        for impl in ORDEN:
            hs = [(1, "E1"), (4, "E4-compacto"), (8, "E8-compacto"), (12, "E12-compacto"), (16, "E16")]
            y = [mediana(filas, experimento="afinidad", impl=impl, n=n, u=u, colocacion=c) for _, c in hs]
            if all(v is not None for v in y):
                ax.plot([100 * h / 16 for h, _ in hs], y, color=COL[impl], marker=MARCA[impl], ms=6, mec=FONDO, mew=0.6)
        ax.set_title(f"Réplica · 4 clústeres E · {titulo_panel(n, u)}", fontsize=10)
        ax.set_xlabel("% de los núcleos físicos en uso")
    for ax in ejes.flat:
        ax.set_ylim(bottom=0)
        ax.set_xticks([0, 25, 50, 75, 100])
    for ax in ejes[:, 0]:
        ax.set_ylabel("rendimiento (Mop/s)")
    leyenda_superior(fig, ORDEN)
    guardar(fig, "miniatura_frente_a_paper.png")


def figura_basura(filas, nombre="memoria_retenida.png"):
    if not filas:
        return
    fig, ax = plt.subplots(figsize=(7.4, 4.3), constrained_layout=True)
    hilos = sorted({int(r["hilos"]) for r in filas})
    for impl in ORDEN:
        y = [mediana(filas, "pico_bloques", impl=impl, hilos=h) for h in hilos]
        if all(v is not None for v in y):
            ax.plot(hilos, y, color=COL[impl], marker=MARCA[impl], ms=7, mec=FONDO, mew=0.6, label=impl)
    lim = [mediana(filas, "limite_bloques", impl="Autores", hilos=h) for h in hilos]
    ax.plot(hilos, lim, color=TINTA2, ls="--", lw=1.4, label="límite sin liberación diferida")
    ax.set_yscale("log")
    ax.set_xlabel("hilos (mitad escriben, mitad leen)")
    ax.set_ylabel("pico de bloques de memoria vivos (escala log.)")
    ax.set_xticks(hilos)
    fig.legend(loc="lower center", bbox_to_anchor=(0.5, 1.0), ncol=3)
    guardar(fig, nombre)


def tabla_basura(filas, salida, sufijo=""):
    if not filas:
        return
    hilos = sorted({int(r["hilos"]) for r in filas})
    salida.append(f"\n## Memoria retenida: pico de bloques vivos{sufijo}\n")
    salida.append("| Hilos | Límite | " + " | ".join(ORDEN) + " |")
    salida.append("|---|---|" + "---|" * len(ORDEN))
    for h in hilos:
        salida.append(f"| {h} | {mediana(filas, 'limite_bloques', impl='Autores', hilos=h):.0f} | " +
                      " | ".join(f"{mediana(filas, 'pico_bloques', impl=i, hilos=h):,.0f}" for i in ORDEN) + " |")


def tabla_rss(filas, salida):
    salida.append("\n## Pico de memoria del proceso (RSS, MB), n = 10⁶, u = 50 %, 24 hilos\n")
    salida.append("| " + " | ".join(ORDEN) + " |")
    salida.append("|" + "---|" * len(ORDEN))
    salida.append("| " + " | ".join(f"{(mediana(filas, 'rss_kb', experimento='fig5', impl=i, n='1000000', u='50', hilos=24) or 0) / 1024:,.0f}" for i in ORDEN) + " |")


def tabla_paridad(filas, salida, nombre):
    par = leer(RES / nombre / "paridad.csv")
    if not par:
        return
    salida.append(f"\n## Validación de los recortes: receta exacta del paper (10 s × 7) frente a 2 s × 5 ({nombre})\n")
    salida.append("| Punto | Impl | 10 s × 7 | 2 s × 5 | Diferencia |")
    salida.append("|---|---|---|---|---|")
    for r in par:
        corta = mediana(filas, experimento="fig5", impl=r["impl"], n=r["n"], u=r["u"], hilos=24)
        larga = float(r["mediana_7x10s"])
        dif = f"{100 * (corta / larga - 1):+.1f} %" if corta else "—"
        salida.append(f"| {titulo_panel(r['n'], r['u'])} | {r['impl']} | {larga:.2f} | {corta:.2f} | {dif} |" if corta else f"| {titulo_panel(r['n'], r['u'])} | {r['impl']} | {larga:.2f} | — | — |")


def tabla_calidad(filas, nombre, salida):
    if not filas:
        return
    fallos = [r for r in filas if r["codigo"].strip() != "0" or r["mops"] == ""]
    temps = [float(r["temp_c"]) for r in filas if r["temp_c"]]
    dispersion = defaultdict(list)
    for r in filas:
        if r["mops"]:
            dispersion[(r["experimento"], r["impl"], r["n"], r["u"], r["hilos"], r["colocacion"])].append(float(r["mops"]))
    cv = sorted(100 * st.pstdev(v) / st.mean(v) for v in dispersion.values() if len(v) > 1 and st.mean(v) > 0)
    salida.append(f"\n## Calidad de las medidas ({nombre})\n")
    salida.append(f"- Ejecuciones: {len(filas)}; fallidas: {len(fallos)}.")
    if temps:
        salida.append(f"- Temperatura: mediana {st.median(temps):.0f} °C, máxima {max(temps):.0f} °C.")
    if cv:
        salida.append(f"- Dispersión entre rondas (coef. de variación): mediana {st.median(cv):.1f} %, percentil 90 {cv[int(len(cv) * 0.9)]:.1f} %.")


def tabla_docker_nativo(docker, nativo, comunes, salida, nombre):
    salida.append(f"\n## Docker frente a {nombre}\n")
    salida.append("Cociente nativo/Docker de las medianas, sobre los 54 puntos de la figura 5 (6 paneles × 9 hilos).\n")
    salida.append("| Impl | Mediana | Mínimo | Máximo |\n|---|---|---|---|")
    for impl in comunes:
        c = []
        for n, u in PANELES:
            for h in (1, 3, 6, 9, 12, 18, 24, 30, 36):
                a = mediana(docker, experimento="fig5", impl=impl, n=n, u=u, hilos=h)
                b = mediana(nativo, experimento="fig5", impl=impl, n=n, u=u, hilos=h)
                if a and b:
                    c.append(b / a)
        if c:
            salida.append(f"| {impl} | {st.median(c):.2f}× | {min(c):.2f}× | {max(c):.2f}× |")


def tabla_controles(filas, salida):
    """Mop/s de las dos variantes de cada control, con su cociente."""
    for control, a, b, titulo in (("alineacion", "sin alinear", "alineado", "Alineación de las pilas en bench.cpp"),
                                  ("docker", "fuera", "dentro", "Mismo código dentro y fuera de Docker")):
        datos = serie(filas, control=control)
        if not datos:
            continue
        salida.append(f"\n## {titulo}\n")
        salida.append(f"Mop/s, mediana de 3 rondas.\n\n| Impl | n | u | Hilos | {a} | {b} | {b} / {a} |\n|---|---|---|---|---|---|---|")
        cocientes = defaultdict(list)
        for clave in sorted({(r["impl"], int(r["n"]), int(r["u"]), int(r["hilos"])) for r in datos}):
            impl, n, u, h = clave
            x = mediana(datos, variante=a, impl=impl, n=n, u=u, hilos=h)
            y = mediana(datos, variante=b, impl=impl, n=n, u=u, hilos=h)
            if x and y:
                salida.append(f"| {impl} | {n} | {u} | {h} | {x:.2f} | {y:.2f} | {y / x:.2f}× |")
                cocientes[impl].append(y / x)
        total = sorted(c for v in cocientes.values() for c in v)
        salida.append(f"\nTodos los cocientes: mediana {st.median(total):.2f}×, mínimo {total[0]:.2f}×, máximo {total[-1]:.2f}×.")


def tabla_williams(filas, salida):
    """bench.cpp dentro de Docker con las dos cabeceras de Williams."""
    if not filas:
        return
    salida.append("\n## Williams en bench.cpp, dentro de Docker: std::atomic frente a boost::atomic\n")
    salida.append("Mop/s, mediana de 3 rondas, n = 1.\n\n| Núcleos | u | Hilos | std::atomic | boost::atomic |\n|---|---|---|---|---|")
    for par, u, h in sorted({(r["par"], int(r["u"]), int(r["hilos"])) for r in filas}):
        x = mediana(filas, variante="std::atomic", par=par, u=u, hilos=h)
        y = mediana(filas, variante="boost::atomic", par=par, u=u, hilos=h)
        salida.append(f"| {par} | {u} | {h} | {x:.2f} | {y:.2f} |")


def tabla_williams_candado(antes, despues, salida):
    """Latencia de Williams con el binario sin -mcx16 (candado oculto) y con el binario corregido."""
    if not antes:
        return
    salida.append("\n## Latencia de Williams: con candado oculto (sin -mcx16) y sin él\n")
    salida.append("Medianas en ns.\n\n| Operación | p | Con candado | Sin candado |\n|---|---|---|---|")
    for op, rotulo in OPS:
        for p in (1, 24, 56):
            a = mediana(antes, "p50", experimento="fig6", impl="Williams", op=op, p=p)
            b = mediana(despues, "p50", experimento="fig6", impl="Williams", op=op, p=p)
            if a and b:
                salida.append(f"| {rotulo} | {p} | {a:.0f} | {b:.0f} |")


def tabla_variabilidad(filas, salida, nombre):
    """Cuánto cambia la misma medición de latencia entre rondas: la ronda más alta entre la más baja."""
    grupos = defaultdict(list)
    for r in serie(filas, experimento="fig6"):
        grupos[(r["impl"], r["op"], int(r["p"]))].append(r)
    if not grupos:
        return
    salida.append(f"\n## Variabilidad de la latencia entre rondas ({nombre})\n")
    salida.append(f"Ronda más alta entre ronda más baja, punto a punto; 1,00× significa que todas coinciden. Hasta {max(map(len, grupos.values()))} rondas.\n")
    salida.append("| p | Medida | Caso típico | Peor caso |\n|---|---|---|---|")
    for p in sorted({k[2] for k in grupos}):
        for campo in ("p50", "p99"):
            cocientes = []
            for clave, v in grupos.items():
                x = [float(r[campo]) for r in v if r[campo]]
                if clave[2] == p and x and min(x) > 0:
                    cocientes.append(max(x) / min(x))
            salida.append(f"| {p} | {campo} | {st.median(cocientes):.2f}× | {max(cocientes):.1f}× |")


def tabla_alineado(antes, despues, salida):
    """La batería sin Docker, sin alinear y con alignas(128) en Pila."""
    salida.append("\n## Batería sin Docker con las pilas alineadas\n")
    salida.append("Mop/s, mediana de la figura 5 con 24 hilos.\n\n| Panel | Impl | Sin alinear | Alineado | Alineado / sin alinear |\n|---|---|---|---|---|")
    for n, u in PANELES:
        for impl in ("Autores", "Libstdc++", "Williams", "Mutex"):
            a = mediana(antes, experimento="fig5", impl=impl, n=n, u=u, hilos=24)
            b = mediana(despues, experimento="fig5", impl=impl, n=n, u=u, hilos=24)
            if a and b:
                salida.append(f"| {titulo_panel(n, u)} | {impl} | {a:.2f} | {b:.2f} | {b / a:.2f}× |")


def trafico(filas, **filtro):
    """GB/s totales por la controladora de memoria."""
    return mediana(filas, "gbs_lectura", **filtro) + mediana(filas, "gbs_escritura", **filtro)


def tabla_memoria(filas, salida):
    """Bytes que pasan de verdad por la controladora de memoria, con n = 10⁶ y u = 50 %."""
    if not filas:
        return
    reposo = trafico(filas, prueba="reposo")
    hilos = sorted({int(r["hilos"]) for r in serie(filas, prueba="pila")})
    salida.append("\n## Tráfico real de RAM con n = 10⁶ y u = 50 % (contadores de la controladora de memoria)\n")
    salida.append(f"En reposo la máquina mueve {reposo:.2f} GB/s, que se restan de la columna de bytes por operación.\n")
    salida.append("Techo medido con `ancho_banda`, en GB/s:\n\n| Prueba | Hilos | Lectura | Escritura | Total |\n|---|---|---|---|---|")
    for prueba in ("lectura", "copia", "azar"):
        for h in sorted({int(r["hilos"]) for r in serie(filas, prueba=prueba)}):
            f = dict(prueba=prueba, hilos=h)
            salida.append(f"| {prueba} | {h} | {mediana(filas, 'gbs_lectura', **f):.1f} | {mediana(filas, 'gbs_escritura', **f):.1f} | {trafico(filas, **f):.1f} |")
    for titulo, celda in (("Mop/s", lambda i, h: f"{mediana(filas, prueba='pila', impl=i, hilos=h):.1f}"),
                          ("GB/s totales", lambda i, h: f"{trafico(filas, prueba='pila', impl=i, hilos=h):.1f}"),
                          ("Bytes de RAM por operación", lambda i, h: f"{1000 * (trafico(filas, prueba='pila', impl=i, hilos=h) - reposo) / mediana(filas, prueba='pila', impl=i, hilos=h):.0f}")):
        salida.append(f"\n{titulo}:\n\n| Hilos | " + " | ".join(ORDEN) + " |\n|---|" + "---|" * len(ORDEN))
        for h in hilos:
            salida.append(f"| {h} | " + " | ".join(celda(i, h) for i in ORDEN) + " |")


def tabla_alineado_memoria(filas, reposo, salida):
    """bench.cpp sin alinear y con alignas(128), alternando, con los contadores de la controladora de memoria."""
    if not filas:
        return
    rondas = sorted({r["ronda"] for r in filas})
    salida.append("\n## Por qué alinear frena con n = 10⁶ (tráfico de RAM, nativo, bench.cpp)\n")
    salida.append(f"24 hilos, u = 50 %, mediana de {len(rondas)} rondas. Las dos versiones de cada punto se midieron seguidas, en orden al azar. "
                  f"A los bytes por operación se les resta el reposo ({reposo:.2f} GB/s).\n")
    salida.append("| n | Impl | Mop/s, sin alinear → alineado | Alineado / sin alinear, por ronda | GB/s | Bytes de RAM por operación |\n|---|---|---|---|---|---|")
    for n in sorted({int(r["n"]) for r in filas}):
        for impl in ("Autores", "Libstdc++", "Williams", "Mutex"):
            f = dict(impl=impl, n=n)
            a, b = (mediana(filas, version=v, **f) for v in ("sin_alinear", "alineado"))
            ga, gb = (trafico(filas, version=v, **f) for v in ("sin_alinear", "alineado"))
            cocientes = [mediana(filas, version="alineado", ronda=r, **f) / mediana(filas, version="sin_alinear", ronda=r, **f) for r in rondas]
            salida.append(f"| {({100000: '10⁵', 1000000: '10⁶'}).get(n, n)} | {impl} | {a:.1f} → {b:.1f} | {min(cocientes):.2f}× a {max(cocientes):.2f}× | "
                          f"{ga:.1f} → {gb:.1f} | {1000 * (ga - reposo) / a:.0f} → {1000 * (gb - reposo) / b:.0f} |")


def figura_memoria(filas):
    if not filas:
        return
    reposo = trafico(filas, prueba="reposo")
    techo, azar = (max(trafico(filas, prueba=p, hilos=h) for h in {r["hilos"] for r in serie(filas, prueba=p)}) for p in ("lectura", "azar"))
    hilos = sorted({int(r["hilos"]) for r in serie(filas, prueba="pila")})
    fig, (izq, der) = plt.subplots(1, 2, figsize=(10.5, 4.2), constrained_layout=True)
    for impl in ORDEN:
        gbs = [trafico(filas, prueba="pila", impl=impl, hilos=h) for h in hilos]
        mops = [mediana(filas, prueba="pila", impl=impl, hilos=h) for h in hilos]
        estilo = dict(color=COL[impl], marker=MARCA[impl], ms=7, mec=FONDO, mew=0.6)
        izq.plot(hilos, gbs, **estilo)
        der.plot(hilos, [1000 * (g - reposo) / m for g, m in zip(gbs, mops)], **estilo)
    izq.axhline(techo, color=TINTA2, ls="--", lw=1.2)
    izq.text(hilos[0], techo, f"techo en orden: {techo:.0f} GB/s", fontsize=8, color=TINTA2, va="bottom")
    izq.axhline(azar, color=TINTA2, ls=":", lw=1.2)
    izq.text(hilos[0], azar, f"techo al azar: {azar:.0f} GB/s", fontsize=8, color=TINTA2, va="bottom")
    izq.set_ylim(0, techo * 1.12)
    izq.set_ylabel("tráfico de RAM (GB/s)")
    der.set_ylim(bottom=0)
    der.set_ylabel("bytes de RAM por operación")
    for ax in (izq, der):
        ax.set_xlabel("número de hilos")
        ax.set_xticks(hilos)
    leyenda_superior(fig, ORDEN)
    guardar(fig, "memoria_ram.png")


def figura_docker_nativo(docker, nativo, comunes, rotulo, nombre):
    fig, ax = plt.subplots(figsize=(6.2, 5.4), constrained_layout=True)
    xs, ys = [], []
    for impl in comunes:
        px, py = [], []
        for n, u in PANELES:
            for h in (1, 3, 6, 9, 12, 18, 24, 30, 36):
                a = mediana(docker, experimento="fig5", impl=impl, n=n, u=u, hilos=h)
                b = mediana(nativo, experimento="fig5", impl=impl, n=n, u=u, hilos=h)
                if a and b:
                    px.append(a), py.append(b)
        ax.plot(px, py, ls="", color=COL[impl], marker=MARCA[impl], ms=6, mec=FONDO, mew=0.4, alpha=0.85, label=impl)
        xs += px
        ys += py
    if not xs:
        plt.close(fig)
        return
    lim = (min(xs + ys) * 0.7, max(xs + ys) * 1.4)
    ax.plot(lim, lim, color=TINTA2, lw=1, ls="--", label="igual en ambos")
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlim(lim)
    ax.set_ylim(lim)
    ax.set_xlabel("Docker · binario oficial (Mop/s)")
    ax.set_ylabel(rotulo)
    ax.legend(loc="upper left")
    guardar(fig, nombre)


def main():
    docker = leer(RES / "docker" / "rendimiento.csv")
    lat = leer(RES / "docker" / "latencia.csv")
    basura = leer(RES / "docker" / "basura.csv")
    nativo = leer(RES / "nativo" / "rendimiento.csv")
    lat_nat = leer(RES / "nativo" / "latencia.csv")
    nat_oficial = leer(RES / "nativo_oficial" / "rendimiento.csv")
    lat_oficial = leer(RES / "nativo_oficial" / "latencia.csv")
    basura_oficial = leer(RES / "nativo_oficial" / "basura.csv")
    nat_alineado = leer(RES / "nativo_alineado" / "rendimiento.csv")
    memoria = leer(RES / "memoria_ram.csv")
    alineado_memoria = leer(RES / "nativo_alineado" / "memoria_alineado.csv")
    paper = leer(AQUI / "paper_fig5.csv")
    salida = [f"# Tablas generadas por analizar.py\n\nDatos: `{RES.name}`."]

    figura_topologia()
    cl = clases_topologia()
    if cl:
        salida.append("\n## Latencia entre núcleos (mediana por clase)\n")
        salida.append("| Clase | ns | Respecto a mismo clúster |\n|---|---|---|")
        base = cl["E, mismo clúster"]
        for k in ("E, mismo clúster", "E, clústeres distintos", "P ↔ E", "P ↔ P"):
            salida.append(f"| {k} | {cl[k]:.1f} | {cl[k] / base:.2f}× |")
    if docker:
        figura_rendimiento(docker, ORDEN, "fig5_replica.png")
        tabla_paper_replica(docker, paper, salida)
        tabla_un_hilo(docker, paper, salida)
        figura_comparacion(docker, paper)
        tabla_rss(docker, salida)
        tabla_paridad(docker, salida, "docker")
        tabla_calidad(docker, "Docker", salida)
        if any(r["experimento"] == "afinidad" for r in docker):
            figura_dosis(docker, ORDEN, "afinidad_dosis_respuesta.png")
            tabla_compacto_disperso(docker, ORDEN, salida, "Docker, binarios oficiales")
            figura_mapa_distancia(docker, ORDEN, 4, "afinidad_mapa_4hilos.png")
            figura_mapa_distancia(docker, ORDEN, 8, "afinidad_mapa_8hilos.png")
            figura_miniatura(docker, paper)
        tabla_pares(docker, ORDEN, salida, "Docker, binarios oficiales")
    if lat:
        figura_latencia(lat, ["Autores", "Libstdc++", "Folly", "Anderson", "Williams"], 24, "fig6_replica.png")
        tabla_latencia(lat, salida)
        tabla_colas(lat, salida)
        tabla_variabilidad(lat, salida, "Docker")
    figura_basura(basura)
    tabla_basura(basura, salida)
    if nativo:
        eq = ["Autores", "Libstdc++", "Williams", "Mutex"]
        figura_rendimiento(nativo, eq, "nativo_fig5.png")
        tabla_calidad(nativo, "Nativo", salida)
        comunes = ["Autores", "Libstdc++", "Williams"]
        figura_docker_nativo(docker, nativo, comunes, "Nativo · bench.cpp del equipo (Mop/s)", "docker_frente_a_nativo.png")
        tabla_docker_nativo(docker, nativo, comunes, salida, "nativo (bench.cpp del equipo)")
        tabla_pares(nativo, eq, salida, "nativo, bench.cpp del equipo")
        if any(r["experimento"] == "afinidad" for r in nativo):
            figura_dosis(nativo, eq, "nativo_dosis_respuesta.png")
            tabla_compacto_disperso(nativo, eq, salida, "nativo, bench.cpp del equipo")
    if lat_nat:
        figura_latencia(lat_nat, ["Autores", "Libstdc++", "Williams", "Mutex"], 24, "nativo_fig6.png")
        tabla_variabilidad(lat_nat, salida, "nativo, bench.cpp del equipo")
    tabla_controles(leer(RES / "controles.csv"), salida)
    tabla_williams(leer(RES / "control_williams.csv"), salida)
    tabla_williams_candado(leer(RES / "docker" / "latencia_williams_con_candado.csv"), lat, salida)
    if nat_oficial:
        figura_rendimiento(nat_oficial, ORDEN, "nativo_oficial_fig5.png")
        tabla_calidad(nat_oficial, "Nativo, binarios oficiales", salida)
        figura_docker_nativo(docker, nat_oficial, ORDEN, "Nativo · binarios oficiales (Mop/s)", "docker_frente_a_nativo_oficial.png")
        tabla_docker_nativo(docker, nat_oficial, ORDEN, salida, "nativo (binarios oficiales)")
        tabla_paridad(nat_oficial, salida, "nativo_oficial")
        tabla_pares(nat_oficial, ORDEN, salida, "nativo, binarios oficiales")
        if any(r["experimento"] == "afinidad" for r in nat_oficial):
            figura_dosis(nat_oficial, ORDEN, "nativo_oficial_dosis_respuesta.png")
            tabla_compacto_disperso(nat_oficial, ORDEN, salida, "nativo, binarios oficiales")
    if lat_oficial:
        figura_latencia(lat_oficial, ["Autores", "Libstdc++", "Folly", "Anderson", "Williams"], 24, "nativo_oficial_fig6.png")
        tabla_latencia(lat_oficial, salida, "nativo, binarios oficiales")
        tabla_colas(lat_oficial, salida, " (nativo, binarios oficiales)")
        tabla_variabilidad(lat_oficial, salida, "nativo, binarios oficiales")
    if basura_oficial:
        figura_basura(basura_oficial, "nativo_oficial_memoria_retenida.png")
        tabla_basura(basura_oficial, salida, " (nativo, binarios oficiales)")
    if nat_alineado:
        figura_rendimiento(nat_alineado, ["Autores", "Libstdc++", "Williams", "Mutex"], "nativo_alineado_fig5.png")
        tabla_calidad(nat_alineado, "Nativo, pilas alineadas", salida)
        tabla_alineado(nativo, nat_alineado, salida)
    figura_memoria(memoria)
    tabla_memoria(memoria, salida)
    tabla_alineado_memoria(alineado_memoria, trafico(memoria, prueba="reposo") if memoria else 0, salida)
    (RES / "tablas.md").write_text("\n".join(salida) + "\n", encoding="utf-8")
    print("tablas.md escrito")


if __name__ == "__main__":
    main()
