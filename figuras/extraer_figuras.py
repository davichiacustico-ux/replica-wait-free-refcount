"""Recorta del PDF del paper las figuras 1-6 y los algoritmos 1-3 que usan las diapositivas (PDF vectorial + PNG).
No se suben al repositorio: cada uno las genera con su copia del paper.
Uso (desde la raíz del repositorio):  python figuras/extraer_figuras.py "ruta/al/paper.pdf"
El PDF esperado es la versión de 17 páginas del repositorio de Utrecht (la 1.ª página es la portada del repositorio)."""
import sys
from pathlib import Path

import pymupdf

# página (0 = portada del repositorio) y rectángulo en puntos
FIGURAS = {
    "fig1": (3, [44, 452, 400, 588]), "fig2": (5, [50, 450, 396, 588]), "alg1": (6, [38, 203, 402, 342]),
    "fig3": (7, [50, 44, 214, 140]), "fig4": (7, [220, 52, 402, 140]), "alg2": (8, [38, 51, 402, 192]),
    "alg3": (9, [52, 313, 400, 467]), "fig5": (11, [52, 48, 400, 278]), "fig6": (12, [36, 50, 388, 245]),
}

if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    src = pymupdf.open(sys.argv[1])
    if src.page_count != 17:
        print(f"aviso: el PDF tiene {src.page_count} páginas (se esperaban 17); revisa que los recortes caigan bien")
    salida = Path(__file__).resolve().parent
    for nombre, (pagina, r) in FIGURAS.items():
        clip = pymupdf.Rect(*r)
        out = pymupdf.open()
        hoja = out.new_page(width=clip.width, height=clip.height)
        hoja.show_pdf_page(hoja.rect, src, pagina, clip=clip)
        out.set_metadata({})
        out.save(salida / f"paper_{nombre}.pdf", garbage=4, deflate=True)
        src[pagina].get_pixmap(dpi=150, clip=clip).save(salida / f"paper_{nombre}.png")
    print(f"{len(FIGURAS)} figuras en {salida}")
