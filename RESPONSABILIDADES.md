# Responsabilidades de la réplica

Réplica de «A Fast Wait-Free Solution to Read-Reclaim Races in Reference Counting» (de Wolff, Anderson, Keller y
Seletskiy, Euro-Par 2024). Cada uno comprueba una afirmación del paper. Lo imprescindible lo llevan David y Jheremy;
las tareas de Diego son autocontenidas y cada una tiene un respaldo, así que nada se bloquea si no llegan.

## Qué tiene que leer cada uno

- **[RESPONSABILIDADES.md](RESPONSABILIDADES.md)**, en la portada del repositorio: la tabla del reparto, el calendario del 2 al 22 de noviembre, los respaldos de cada tarea de Diego y las reglas de trabajo.
- **Su incidencia**, con la lista de tareas marcable por semana:
  - [#1 David: corrección y latencia](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/1)
  - [#2 Jheremy: rendimiento e integración](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/2)
  - [#3 Diego: memoria, tercer equipo y comparación](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/3)
- **Su rama** ([`david`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/david), [`jheremy`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/jheremy), [`diego`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/diego)) para trabajar. Los *pull requests* a `main` los integra Jheremy.
- **Su apartado del informe**, ya creado en [`replica/informe/`](replica/informe/) con encabezados para rellenar.

## Tabla del reparto

| | David Tamo Turpo | Jheremy Manuel Alvarez Astete | Diego Quispe Candelario |
|---|---|---|---|
| **Incidencia / Rama** | [#1](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/1) · rama [`david`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/david) | [#2](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/2) · rama [`jheremy`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/jheremy) | [#3](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/3) · rama [`diego`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/diego) |
| **Afirmación** | «Es correcto y wait-free» + «menor latencia de lectura» | «Es el más rápido de los que no gastan memoria extra» | «Sin sobrecoste de memoria» + ¿se mantiene en otro equipo? |
| **Experimentos** | Corrección (ASan + ThreadSanitizer, variantes rotas, intentos de CAS ≤ P, simulador) y latencia (fig. 6) | Rendimiento (fig. 5), 6 paneles × 3 repeticiones | Memoria (pico de RSS) y la batería completa en su PC |
| **Archivos** | `codigo/estres_asan.cpp`, `codigo/boceto/`, `simulador/`, `codigo/bench.cpp latencia` | `codigo/bench.cpp`, `codigo/ejecutar_pruebas.sh`, `graficos/hacer_graficos.py` | `codigo/medir_memoria.sh` (nuevo), `replica/resultados/diego-<cpu>/` |
| **Informe** | `replica/informe/01_corrección.md`, `02_latencia.md` | `replica/informe/03_rendimiento.md` e integración final | `replica/informe/04_memoria_y_tercer_equipo.md`, `05_comparación.md` |
| **Exposición** | Problema, corrección y latencia (~10 min) | Rendimiento y conclusiones (~8 min) | Memoria, tercer equipo y comparación (~4 min) |

## Calendario (ajustadlo a la fecha de entrega)

| Semana | David | Jheremy | Diego |
|---|---|---|---|
| 1 · 2–8 nov | Añadir ThreadSanitizer; ampliar hilos y operaciones de `estres_asan` | Fijar `bench.cpp` y el script (interfaz, parámetros, repeticiones) | **Entregar** `replica/resultados/diego-<cpu>/` (ejecutar el script en su PC) |
| 2 · 9–15 nov | Instrumentar el máximo de intentos de CAS; medir latencia con p = 1 y p = todos | Ejecuciones completas de rendimiento con 3 repeticiones | **Entregar** la medida de memoria (mitad de semana) |
| 3 · 16–22 nov | Gráficas y apartados 01 y 02 | Gráficas, apartado 03 e integración del informe | **Entregar** la tabla «paper frente a réplica» (inicio de semana) y el apartado 04–05 |

## Respaldos (si una tarea de Diego no llega a tiempo)

| Tarea de Diego | Plazo | Si no llega |
|---|---|---|
| Batería en su PC | fin de la semana 1 | la réplica queda en dos equipos (David y Jheremy) y se dice así en el informe |
| Memoria (RSS) | mitad de la semana 2 | la mide Jheremy con las mismas ejecuciones de `bench` |
| Tabla comparativa | inicio de la semana 3 | la hace David |
| Su parte de la exposición | día de la exposición | la presenta Jheremy (son pocas diapositivas, basadas en tablas) |

## Reglas de trabajo y notas del repositorio

- **Entorno común**: Linux o WSL con Ubuntu 24.04 y g++ ≥ 12 (C++20). Antes de medir: `bash codigo/ejecutar_pruebas.sh` sin cambios debe funcionar.
- **Cada equipo deja sus resultados en su carpeta**: `cd codigo && bash ejecutar_pruebas.sh ../replica/resultados/<nombre>-<cpu>` (por ejemplo `david-i5-10400`). Anotad CPU, núcleos y si es WSL o Linux nativo.
- **Una rama por persona** (`david`, `jheremy`, `diego`) y *pull request* a `main`; Jheremy revisa e integra.
- **No cambiar parámetros de `bench.cpp` sin avisar**: lo usan David (latencia) y Jheremy (rendimiento).
- **Repeticiones**: 3 por punto; se informa la mediana y el rango.
- **Recortes acordados**: hilos hasta ×1,5 los núcleos; sin rivales nuevos (Folly, Anderson) — van como trabajo futuro; la memoria solo con el pico de RSS.
- **Submódulo de los autores**: el código original de los autores va enlazado como submódulo en `codigo/autores` (fijado al commit `8540c58`), no copiado, porque su repositorio no declara licencia. Hay que clonar con `git clone --recursive`.
- **Figuras del paper**: las figuras del paper no se suben al repositorio; `python figuras/extraer_figuras.py "ruta/al/paper.pdf"` las regenera localmente para compilar las diapositivas.

## Tareas de cada uno, en detalle

### David ([#1](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/1) · rama [`david`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/david))
- [ ] `estres_asan.cpp`: versión con `-fsanitize=thread`; más hilos (8, 12, 16) y más operaciones.
- [ ] Instrumentar una copia del código de los autores para contar intentos de CAS en `increase_weight` y comprobar el máximo ≤ P.
- [ ] Repetir las variantes rotas del boceto (`SIN_SALIDA`, `VAR_C=8`) y el simulador (`node simulador/buscar.js 20000 ingenua,umbral0,sinReserva,sinSalida`).
- [ ] Latencia: `./bin/bench latencia 100000` con 3 repeticiones; diagramas de caja como la fig. 6.
- [ ] Apartados `01_corrección.md` y `02_latencia.md`.

### Jheremy ([#2](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/2) · rama [`jheremy`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/jheremy))
- [ ] Revisar `bench.cpp` y dejar fijos hilos, duración (1 s) y repeticiones (3); que el script acepte la carpeta de salida (ya lo hace).
- [ ] Rendimiento: los 6 paneles × 3 repeticiones; gráfica como la fig. 5 con mediana y rango.
- [ ] Comparar con el paper (tendencias, no cifras absolutas: 12 hilos frente a 144).
- [ ] Apartado `03_rendimiento.md` e integrar el informe completo.

### Diego ([#3](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/3) · rama [`diego`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/diego))
- [ ] Ejecutar la batería en su PC y subir `replica/resultados/diego-<cpu>/`.
- [ ] Memoria: para cada implementación, `/usr/bin/time -v ./bin/bench rendimiento 1.0` (o un modo reducido) y anotar el *Maximum resident set size*; guardarlo en `codigo/medir_memoria.sh` y su salida en `replica/resultados/`.
- [ ] Tabla «paper frente a réplica» (afirmación, resultado del paper, resultado en cada equipo, ¿se reproduce?).
- [ ] Apartados `04_memoria_y_tercer_equipo.md` y `05_comparación.md`.
