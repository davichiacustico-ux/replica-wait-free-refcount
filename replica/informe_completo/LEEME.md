# Informe actualizado de la réplica (Core Ultra 9 285 + Core i5-10400)

Esta carpeta contiene la versión completa y actualizada del informe escrito de la réplica de **«A Fast Wait-Free Solution to Read-Reclaim Races in Reference Counting»** (de Wolff, Anderson, Keller y Seletskiy, Euro-Par 2024), conservando intactos los borradores preliminares del 2 de noviembre en [`../informe/`](../informe/).

## Contenido de la carpeta

| Archivo / carpeta | Qué contiene |
|---|---|
| [`informe_replica_ultra9.tex`](informe_replica_ultra9.tex) | Fuente LaTeX completa del informe (1 columna A4, `booktabs`, BibTeX, tablas y figuras vectoriales de ambas máquinas) |
| [`informe_replica_ultra9.pdf`](informe_replica_ultra9.pdf) | PDF compilado con `pdflatex` + `bibtex` y metadatos limpios |
| [`referencias.bib`](referencias.bib) | Bibliografía BibTeX verificada (24 entradas: paper Euro-Par 2024, artefacto Zenodo, rivales, artículos fundacionales y trabajo relacionado 2024) |
| `figuras/` | 17 figuras vectoriales `.pdf` autocontenidas (traídas desde `guias-wait-free-refcount/presentacion_resultados/figuras/`) para compilar el informe sin dependencias externas |
| [`01_correccion.md`](01_correccion.md) | Apartado 1 actualizado: ASan, UBSan y TSan en ambas máquinas y sobre los rivales, variantes rotas, simulador y cota *wait-free* instrumentada en el código real ($P = 65\,536$ y $P = 32$) |
| [`02_latencia.md`](02_latencia.md) | Apartado 2 actualizado: Figura 6 con binarios oficiales (con y sin Docker, 5 y 15 rondas), corrección de `99,7 µs`, candado oculto de Williams (`-mcx16`), colas `p99,95` y contraste con `bench.cpp` |
| [`03_rendimiento.md`](03_rendimiento.md) | Apartado 3 actualizado: Figura 5 con los 5 algoritmos oficiales, Docker frente a nativo, brecha de Folly a 1 hilo, validación `10 s × 7`, distancia entre clústeres y alineación `alignas(128)` |
| [`04_memoria_y_segundo_equipo.md`](04_memoria_y_segundo_equipo.md) | Apartado 4 actualizado: bloques retenidos (`bench_basura`), RSS en $n = 10^6$, tráfico real de RAM por controladora IMC y comparativa completa Core Ultra 9 285 frente a Core i5-10400 |
| [`05_comparacion.md`](05_comparacion.md) | Apartado 5 actualizado: veredicto de las 15 afirmaciones del paper (9 se reproducen, 2 en parte, 3 no se reproducen, 1 no se confirma), hallazgos propios y limitaciones |

## Cómo compilar el PDF

Desde esta carpeta (`replica/informe_completo/`):

```bash
latexmk -pdf -interaction=nonstopmode -halt-on-error informe_replica_ultra9.tex
# o paso a paso:
pdflatex informe_replica_ultra9.tex
bibtex informe_replica_ultra9
pdflatex informe_replica_ultra9.tex
pdflatex informe_replica_ultra9.tex
```

Todas las cifras provienen exclusivamente de [`../completa/EXPLICACION.md`](../completa/EXPLICACION.md), [`../resultados/david-ultra9-285/tablas.md`](../resultados/david-ultra9-285/tablas.md) y los CSV de `../resultados/david-ultra9-285/` y `../../capturas/` (sin usar `latencia_williams_con_candado.csv` salvo para documentar el error corregido).
