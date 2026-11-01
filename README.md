# Réplica: conteo de referencias wait-free (Euro-Par 2024)

Exposición y réplica del paper **«A Fast Wait-Free Solution to Read-Reclaim Races in Reference Counting»**
(I. G. de Wolff, D. Anderson, G. K. Keller y A. Seletskiy, Euro-Par 2024, LNCS 14803, págs. 103–118,
doi:10.1007/978-3-031-69583-4_8).

Equipo: David Tamo Turpo, Jheremy Manuel Alvarez Astete y Diego Quispe Candelario.

## Qué tiene que leer cada uno

- **[RESPONSABILIDADES.md](RESPONSABILIDADES.md)**, en la portada del repositorio: la tabla del reparto, el calendario del 2 al 22 de noviembre, los respaldos de cada tarea de Diego y las reglas de trabajo.
- **Su incidencia**, con la lista de tareas marcable por semana:
  - [#1 David: corrección y latencia](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/1)
  - [#2 Jheremy: rendimiento e integración](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/2)
  - [#3 Diego: memoria, tercer equipo y comparación](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/3)
- **Su rama** ([`david`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/david), [`jheremy`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/jheremy), [`diego`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/diego)) para trabajar. Los *pull requests* a `main` los integra Jheremy.
- **Su apartado del informe**, ya creado en [`replica/informe/`](replica/informe/) con encabezados para rellenar.

## División de responsabilidades

Cada uno comprueba una afirmación del paper. Lo imprescindible lo llevan David y Jheremy; las tareas de Diego son autocontenidas y cada una tiene un respaldo, así que nada se bloquea si no llegan.

| | David Tamo Turpo | Jheremy Manuel Alvarez Astete | Diego Quispe Candelario |
|---|---|---|---|
| **Incidencia / Rama** | [#1](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/1) · rama [`david`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/david) | [#2](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/2) · rama [`jheremy`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/jheremy) | [#3](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/3) · rama [`diego`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/diego) |
| **Afirmación** | «Es correcto y wait-free» + «menor latencia de lectura» | «Es el más rápido de los que no gastan memoria extra» | «Sin sobrecoste de memoria» + ¿se mantiene en otro equipo? |
| **Experimentos** | Corrección (ASan + ThreadSanitizer, variantes rotas, intentos de CAS ≤ P, simulador) y latencia (fig. 6) | Rendimiento (fig. 5), 6 paneles × 3 repeticiones | Memoria (pico de RSS) y la batería completa en su PC |
| **Archivos** | `codigo/estres_asan.cpp`, `codigo/boceto/`, `simulador/`, `codigo/bench.cpp latencia` | `codigo/bench.cpp`, `codigo/ejecutar_pruebas.sh`, `graficos/hacer_graficos.py` | `codigo/medir_memoria.sh` (nuevo), `replica/resultados/diego-<cpu>/` |
| **Informe** | `replica/informe/01_corrección.md`, `02_latencia.md` | `replica/informe/03_rendimiento.md` e integración final | `replica/informe/04_memoria_y_tercer_equipo.md`, `05_comparación.md` |
| **Exposición** | Problema, corrección y latencia (~10 min) | Rendimiento y conclusiones (~8 min) | Memoria, tercer equipo y comparación (~4 min) |

### Calendario (2 al 22 de noviembre)

| Semana | David | Jheremy | Diego |
|---|---|---|---|
| 1 · 2–8 nov | Añadir ThreadSanitizer; ampliar hilos y operaciones de `estres_asan` | Fijar `bench.cpp` y el script (interfaz, parámetros, repeticiones) | **Entregar** `replica/resultados/diego-<cpu>/` (ejecutar el script en su PC) |
| 2 · 9–15 nov | Instrumentar el máximo de intentos de CAS; medir latencia con p = 1 y p = todos | Ejecuciones completas de rendimiento con 3 repeticiones | **Entregar** la medida de memoria (mitad de semana) |
| 3 · 16–22 nov | Gráficas y apartados 01 y 02 | Gráficas, apartado 03 e integración del informe | **Entregar** la tabla «paper frente a réplica» (inicio de semana) y el apartado 04–05 |

### Respaldos (si una tarea de Diego no llega a tiempo)

| Tarea de Diego | Plazo | Si no llega |
|---|---|---|
| Batería en su PC | fin de la semana 1 | la réplica queda en dos equipos (David y Jheremy) y se dice así en el informe |
| Memoria (RSS) | mitad de la semana 2 | la mide Jheremy con las mismas ejecuciones de `bench` |
| Tabla comparativa | inicio de la semana 3 | la hace David |
| Su parte de la exposición | día de la exposición | la presenta Jheremy (son pocas diapositivas, basadas en tablas) |

### Reglas de trabajo

- **Entorno común**: Linux o WSL con Ubuntu 24.04 y g++ ≥ 12 (C++20). Antes de medir: `bash codigo/ejecutar_pruebas.sh` sin cambios debe funcionar.
- **Cada equipo deja sus resultados en su carpeta**: `cd codigo && bash ejecutar_pruebas.sh ../replica/resultados/<nombre>-<cpu>` (por ejemplo `david-i5-10400`). Anotad CPU, núcleos y si es WSL o Linux nativo.
- **Una rama por persona** (`david`, `jheremy`, `diego`) y *pull request* a `main`; Jheremy revisa e integra.
- **No cambiar parámetros de `bench.cpp` sin avisar**: lo usan David (latencia) y Jheremy (rendimiento).
- **Repeticiones**: 3 por punto; se informa la mediana y el rango.
- **Recortes acordados**: hilos hasta ×1,5 los núcleos; sin rivales nuevos (Folly, Anderson) — van como trabajo futuro; la memoria solo con el pico de RSS.

### Tareas de cada uno, en detalle

#### David ([#1](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/1) · rama [`david`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/david))
- [ ] `estres_asan.cpp`: versión con `-fsanitize=thread`; más hilos (8, 12, 16) y más operaciones.
- [ ] Instrumentar una copia del código de los autores para contar intentos de CAS en `increase_weight` y comprobar el máximo ≤ P.
- [ ] Repetir las variantes rotas del boceto (`SIN_SALIDA`, `VAR_C=8`) y el simulador (`node simulador/buscar.js 20000 ingenua,umbral0,sinReserva,sinSalida`).
- [ ] Latencia: `./bin/bench latencia 100000` con 3 repeticiones; diagramas de caja como la fig. 6.
- [ ] Apartados `01_corrección.md` y `02_latencia.md`.

#### Jheremy ([#2](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/2) · rama [`jheremy`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/jheremy))
- [ ] Revisar `bench.cpp` y dejar fijos hilos, duración (1 s) y repeticiones (3); que el script acepte la carpeta de salida (ya lo hace).
- [ ] Rendimiento: los 6 paneles × 3 repeticiones; gráfica como la fig. 5 con mediana y rango.
- [ ] Comparar con el paper (tendencias, no cifras absolutas: 12 hilos frente a 144).
- [ ] Apartado `03_rendimiento.md` e integrar el informe completo.

#### Diego ([#3](https://github.com/davichiacustico-ux/replica-wait-free-refcount/issues/3) · rama [`diego`](https://github.com/davichiacustico-ux/replica-wait-free-refcount/tree/diego))
- [ ] Ejecutar la batería en su PC y subir `replica/resultados/diego-<cpu>/`.
- [ ] Memoria: para cada implementación, `/usr/bin/time -v ./bin/bench rendimiento 1.0` (o un modo reducido) y anotar el *Maximum resident set size*; guardarlo en `codigo/medir_memoria.sh` y su salida en `replica/resultados/`.
- [ ] Tabla «paper frente a réplica» (afirmación, resultado del paper, resultado en cada equipo, ¿se reproduce?).
- [ ] Apartados `04_memoria_y_tercer_equipo.md` y `05_comparación.md`.

---

## Clonar

```bash
git clone --recursive https://github.com/davichiacustico-ux/replica-wait-free-refcount.git
# si ya lo clonaste sin --recursive:
git submodule update --init
```

`codigo/autores` es un submódulo con el código original de los autores
([ivogabe/wait-free-atomic-shared-pointer](https://github.com/ivogabe/wait-free-atomic-shared-pointer), commit 8540c58).
No se copia aquí porque su repositorio no declara licencia.

## Estructura

| Carpeta / archivo | Qué hay |
|---|---|
| `exposicion_15min.tex`, `exposicion_25min.tex` | Diapositivas (los PDF no se publican porque incluyen figuras del paper: se compilan en local tras `figuras/extraer_figuras.py`) |
| `guion_15min.pdf`, `guion_25min.pdf`, `guion_expositores.pdf` | Qué decir en cada diapositiva |
| `codigo/` | Pruebas: `estres_asan.cpp` (la carrera con AddressSanitizer), `boceto/` (versión didáctica y variantes rotas), `bench.cpp` (rendimiento y latencia), `ejecutar_pruebas.sh` |
| `simulador/` | Simulador de intercalados que comprueba los invariantes del paper paso a paso (Node.js) |
| `capturas/` | Salidas de referencia (WSL Ubuntu 24.04, Intel i5-10400, 12 hilos) |
| `graficos/` | `hacer_graficos.py`: gráficas de rendimiento y latencia y capturas de terminal como PDF |
| `figuras/` | `extraer_figuras.py`: recorta del PDF del paper las figuras de las diapositivas (no se suben) |
| `practica_cpp/` | Lo mínimo de C/C++ para entender el paper, en 8 partes con ejercicios |
| `replica/informe/` | Un apartado por persona |
| `replica/resultados/` | Resultados de cada equipo: `<nombre>-<cpu>/` |

## Ejecutar las pruebas (Linux o WSL, g++ ≥ 12)

```bash
cd codigo
bash ejecutar_pruebas.sh                                   # salida en ../capturas
bash ejecutar_pruebas.sh ../replica/resultados/david-i5-10400   # salida en tu carpeta de resultados
```

El script compila y guarda: demo de los autores, AddressSanitizer (versión ingenua frente a la de los autores),
boceto y variantes, y `bench` en modo rendimiento y latencia (CSV). Tarda unos 3 minutos.

```bash
node simulador/buscar.js 20000 ingenua,sinReserva          # simulador de contraejemplos
python graficos/hacer_graficos.py                          # gráficas y capturas como PDF
python figuras/extraer_figuras.py "ruta/al/paper.pdf"      # figuras del paper para compilar las diapositivas
latexmk -pdf exposicion_25min.tex                          # diapositivas (MiKTeX o TeX Live)
```

## Resultados de referencia (i5-10400, 12 hilos)

- AddressSanitizer: la versión ingenua produce *heap-use-after-free*; el código de los autores, 1,6 M operaciones sin errores.
- Rendimiento con 1 pila, 1 % de actualizaciones y 12 hilos: autores 34,3 Mop/s, `std::atomic<std::shared_ptr>` de GCC 2,2, mutex 6,4.
- Latencia de `load` con 12 hilos (mediana / p99): autores 269 ns / 0,83 µs; GCC 3,06 µs / 16 µs; mutex 61 ns / 27,6 µs.
