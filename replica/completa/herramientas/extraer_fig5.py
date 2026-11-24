#!/usr/bin/env python3
"""Recupera los valores numéricos de la figura 5 del paper a partir de sus gráficos vectoriales.

La figura es vectorial. Las líneas no sirven, porque matplotlib borra los puntos casi alineados al guardar;
los marcadores sí están todos (6 paneles x 9 puntos, más el de la leyenda, por cada color). Se toma el centro
de cada marcador y se traduce a Mop/s con las líneas de cuadrícula de su panel.

Uso: pdftocairo -svg -f 12 -l 12 paper.pdf pagina12.svg
     python3 extraer_fig5.py pagina12.svg > paper_fig5.csv
"""
import re
import sys

COLORES = {(83.9, 16.1, 16.1): "Autores", (14.9, 46.7, 70.6): "Libstdc++", (17.6, 62.7, 18.4): "Williams",
           (95.3, 49.4, 14.9): "Folly", (58.4, 40.8, 74.1): "Anderson"}
HILOS = [1, 18, 36, 54, 72, 108, 144, 180, 216]
# los paneles en orden de lectura, con las marcas de su eje y
PANELES = [((1, 1), [0, 3, 6, 9]), ((1, 10), [0, 2, 4, 6]), ((1, 50), [0, 1, 2, 3]),
           ((10, 1), [0, 20, 40]), ((10, 10), [0, 5, 10, 15, 20]), ((1000000, 50), [0, 50, 100])]
CORTE_FILAS = 168   # coordenada y que separa las dos filas de paneles en la página


def rgb(texto):
    return tuple(round(float(x), 1) for x in re.findall(r"([\d.]+)%", texto))


def puntos(atributos):
    """Todos los puntos del trazado, ya en coordenadas de página."""
    d = re.search(r'(?:^| )d="([^"]+)"', atributos).group(1)
    m = re.search(r'transform="matrix\(([^)]+)\)"', atributos)
    a, b, c, e, f, g = (float(x) for x in re.split(r"[ ,]+", m.group(1).strip())) if m else (1, 0, 0, 1, 0, 0)
    numeros = [float(x) for x in re.findall(r"-?\d+\.?\d*(?:e-?\d+)?", d)]
    pares = zip(numeros[0::2], numeros[1::2])
    return [(a * x + c * y + f, b * x + e * y + g) for x, y in pares]


def main():
    svg = open(sys.argv[1], encoding="utf-8").read()
    marcadores, horizontales, verticales = [], [], []
    for atributos in re.findall(r"<path ([^>]*?)/?>", svg):
        if ' d="' not in " " + atributos:
            continue
        relleno = re.search(r'fill="(rgb[^"]+)"', atributos)
        trazo = re.search(r'stroke="(rgb[^"]+)"', atributos)
        if relleno and rgb(relleno.group(1)) in COLORES:
            p = puntos(atributos)
            xs, ys = [q[0] for q in p], [q[1] for q in p]
            marcadores.append((COLORES[rgb(relleno.group(1))], (min(xs) + max(xs)) / 2, (min(ys) + max(ys)) / 2))
        elif trazo and "dasharray" in atributos and rgb(trazo.group(1))[0] > 70:
            p = puntos(atributos)
            if len(p) == 2:
                (horizontales if abs(p[0][1] - p[1][1]) < 0.01 else verticales).append(p)

    columnas = sorted({round(l[0][0]) for l in horizontales})
    print("impl,n,u,hilos,mops")
    for indice, ((n, u), marcas) in enumerate(PANELES):
        fila, columna = divmod(indice, 3)
        izquierda = columnas[columna]
        derecha = max(l[1][0] for l in horizontales if round(l[0][0]) == izquierda)
        lineas = sorted({round(l[0][1], 3) for l in horizontales
                         if round(l[0][0]) == izquierda and (l[0][1] > CORTE_FILAS) == (fila == 1)}, reverse=True)
        assert len(lineas) == len(marcas), ((n, u), lineas, marcas)
        escala = (marcas[-1] - marcas[0]) / (lineas[0] - lineas[-1])
        borde = [l for l in verticales if izquierda <= l[0][0] <= derecha and (l[0][1] > CORTE_FILAS) == (fila == 1)][0]
        arriba, abajo = min(borde[0][1], borde[1][1]), max(borde[0][1], borde[1][1])
        for impl in COLORES.values():
            pts = sorted((x, y) for i, x, y in marcadores if i == impl and izquierda <= x <= derecha and arriba - 2 <= y <= abajo + 2)
            assert len(pts) == len(HILOS), ((n, u), impl, len(pts))
            for h, (_, y) in zip(HILOS, pts):
                print(f"{impl},{n},{u},{h},{marcas[0] + (lineas[0] - y) * escala:.2f}")


if __name__ == "__main__":
    main()
