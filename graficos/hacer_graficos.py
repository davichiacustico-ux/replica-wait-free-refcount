"""Gráficas de la mini-reproducción (estilo de las figuras 5 y 6 del paper) y capturas de terminal como PDF vectorial.
Uso: python graficos/hacer_graficos.py   (desde exposicion_paper/)"""
import csv
import re
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

RAIZ = Path(__file__).resolve().parent.parent
CAP = RAIZ / "capturas"
OUT = RAIZ / "graficos"
META = {"Creator": None, "Producer": None, "Author": None}
COL = {"Autores": "#d62728", "Libstdc++": "#1f77b4", "Mutex": "#555555"}
MARK = {"Autores": "o", "Libstdc++": "s", "Mutex": "^"}
plt.rcParams.update({"font.size": 10, "axes.spines.top": False, "axes.spines.right": False, "axes.grid": True, "grid.alpha": 0.3, "grid.linestyle": ":"})


def rendimiento():
    filas = list(csv.DictReader(open(CAP / "rendimiento.csv", encoding="utf-8")))
    paneles = [("1", "1"), ("1", "10"), ("1", "50"), ("10", "1"), ("10", "10"), ("100000", "50")]
    fig, ejes = plt.subplots(2, 3, figsize=(11, 5.6))
    for ax, (n, u) in zip(ejes.flat, paneles):
        for impl in COL:
            pts = [(int(r["hilos"]), float(r["mops"])) for r in filas if r["impl"] == impl and r["n"] == n and r["u"] == u]
            ax.plot([p[0] for p in pts], [p[1] for p in pts], marker=MARK[impl], color=COL[impl], label=impl, lw=1.8, ms=5)
        ax.axvline(12, color="#999", ls="--", lw=1)
        ax.set_title(f"n = {'10^5' if n == '100000' else n}, u = {u} %", fontsize=11)
        ax.set_xticks([1, 4, 8, 12, 18])
        ax.set_ylim(bottom=0)
    for ax in ejes[1]:
        ax.set_xlabel("número de hilos")
    for ax in ejes[:, 0]:
        ax.set_ylabel("Mop/s")
    ejes[0, 0].legend(loc="upper center", bbox_to_anchor=(1.7, 1.38), ncol=3, frameon=False)
    fig.tight_layout()
    fig.savefig(OUT / "local_rendimiento.pdf", metadata=META, bbox_inches="tight")
    plt.close(fig)


def latencia():
    filas = list(csv.DictReader(open(CAP / "latencia.csv", encoding="utf-8")))
    ops = [("load", "load"), ("store", "store"), ("store&reclaim", "store & reclaim")]
    fig, ejes = plt.subplots(1, 3, figsize=(11, 3.9), sharey=True)
    for ax, (op, tit) in zip(ejes, ops):
        stats = []
        for impl in COL:
            r = next(x for x in filas if x["impl"] == impl and x["op"] == op and x["p"] != "1")
            v = [max(float(r[k]), 10.0) for k in ("p1", "p25", "p50", "p75", "p99")]   # en escala logarítmica, < 10 ns se dibuja en 10
            stats.append({"label": impl, "whislo": v[0], "q1": v[1], "med": v[2], "q3": v[3], "whishi": v[4], "fliers": []})
        b = ax.bxp(stats, showfliers=False, patch_artist=True, widths=0.5, medianprops={"color": "#e8740c", "lw": 2})
        for patch, impl in zip(b["boxes"], COL):
            patch.set_facecolor(COL[impl] + "22"); patch.set_edgecolor(COL[impl])
        for i, s in enumerate(stats):
            med = s["med"]
            ax.annotate(f"{med:.0f} ns" if med < 1000 else f"{med / 1000:.2f} µs".replace(".", ","), (i + 1.3, med), fontsize=8.5, va="center")
        ax.set_yscale("log")
        ax.set_ylim(8, 1e5)
        ax.set_title(f"{tit} (p = 12)", fontsize=11)
    ejes[0].set_ylabel("latencia (ns, escala log.)")
    fig.tight_layout()
    fig.savefig(OUT / "local_latencia.pdf", metadata=META, bbox_inches="tight")
    plt.close(fig)


def abreviar(linea):
    linea = linea.replace("/mnt/c/Users/David/Documents/Semestres/2026B/cpd2/exposicion_paper/codigo/", "")
    linea = re.sub(r" 0x[0-9a-f]+ in ", " ", linea)
    linea = re.sub(r"(/usr/include/c\+\+/13/bits/|\.\./\.\./\.\./\.\./src/libsanitizer/asan/)", "", linea)
    linea = re.sub(r" at pc 0x[0-9a-f]+ bp 0x[0-9a-f]+ sp 0x[0-9a-f]+", "", linea)
    return linea


def captura(nombre, titulo, archivo, filtro=None, ancho=None, fs=10):
    lineas = [abreviar(l.rstrip("\n")) for l in open(CAP / archivo, encoding="utf-8")]
    if filtro:
        lineas = filtro(lineas)
    while lineas and not lineas[-1].strip():
        lineas.pop()
    if ancho is None:   # ancho ajustado a la línea más larga: el texto sale más grande al escalar en la diapositiva
        ancho = max(5.2, min(12.5, 0.6 * fs / 72 * max(len(l) for l in lineas) + 0.45))
    alto_linea = fs * 1.42 / 72
    alto = 0.42 + alto_linea * len(lineas) + 0.18
    fig = plt.figure(figsize=(ancho, alto))
    fig.patch.set_facecolor("#1e1f24")
    ax = fig.add_axes([0, 0, 1, 1]); ax.set_axis_off(); ax.set_xlim(0, ancho); ax.set_ylim(0, alto)
    ax.add_patch(plt.Rectangle((0, alto - 0.36), ancho, 0.36, color="#33353d"))
    for i, c in enumerate(["#ff5f57", "#febc2e", "#28c840"]):
        ax.add_patch(plt.Circle((0.22 + i * 0.2, alto - 0.18), 0.055, color=c))
    ax.text(ancho / 2, alto - 0.18, titulo, color="#c9ccd3", ha="center", va="center", fontsize=9, family="DejaVu Sans")
    y = alto - 0.36 - 0.12 - alto_linea * 0.75
    for l in lineas:
        col = "#e6e6e6"
        if l.startswith("$"):
            col = "#7ee787"
        elif re.search(r"ERROR|FALLA|heap-use-after-free|errores detectados *= [1-9]|USO TRAS|LIBERACIÓN|DESBORDE|PESO NEG", l):
            col = "#ff7b72"
        elif re.search(r"sin errores|= 0$|0 fallos|Paper +0 ", l):
            col = "#a5d6ff"
        ax.text(0.15, y, l, color=col, family="DejaVu Sans Mono", fontsize=fs, va="baseline")
        y -= alto_linea
    fig.savefig(OUT / f"captura_{nombre}.pdf", metadata=META, facecolor=fig.get_facecolor())
    plt.close(fig)


if __name__ == "__main__":
    rendimiento()
    latencia()
    T = "WSL · Ubuntu 24.04 · Intel Core i5-10400 (12 hilos)"
    def asan(ls):
        keep = [ls[0].split(" | grep")[0] + "   # (salida filtrada)"]
        for l in ls[1:]:
            if re.search(r"ERROR|WRITE|freed by|#[0-2] ", l) and "__invoke" not in l:
                keep.append(l.replace("heap-use-after-free on address", "heap-use-after-free en"))
            if l.startswith("previously"):
                break
        return keep
    captura("asan_ingenua", T, "02_asan_ingenua.txt", asan)
    captura("asan_autores", T, "03_asan_autores.txt")
    captura("autores_demo", T, "01_autores_demo.txt")
    captura("boceto", T, "04_boceto.txt")
    captura("boceto_variantes", T, "05_boceto_variantes.txt")
    W = "Windows · Node.js · simulador de intercalados"
    captura("simulador", W, "08_simulador.txt", lambda ls: ls[:12])
    captura("simulador_traza", W, "08_simulador.txt", lambda ls: ["$ node buscar.js 20000 ingenua,sinReserva   # (continuación)"] + ls[24:])
    captura("entorno", T, "00_entorno.txt")
    captura("latencia_txt", T, "07_latencia.txt", lambda ls: ["$ ./bin/bench latencia 100000"] + ls, fs=9)
    print("listo")
