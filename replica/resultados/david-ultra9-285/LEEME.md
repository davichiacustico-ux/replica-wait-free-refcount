# Resultados de david-ultra9-285

Máquina: Intel Core Ultra 9 285, 24 núcleos (8 P y 16 E en 4 clústeres), sin SMT, 1 nodo NUMA. Los detalles están en [equipo.txt](equipo.txt).

Para entender qué significan estos números, lee [../../completa/EXPLICACION.md](../../completa/EXPLICACION.md). Para regenerar las gráficas y tablas: `python3 replica/completa/analizar.py`.

## Resumen de lo que hay

| Qué | Dónde | Filas |
|---|---|---|
| Rendimiento con Docker (programas oficiales, 5 algoritmos) | `docker/rendimiento.csv` | 2 910 |
| Latencia con Docker | `docker/latencia.csv` | 810 |
| Memoria retenida con Docker | `docker/basura.csv` | 75 |
| Receta exacta del paper con Docker (10 s × 7) | `docker/paridad.csv` | 10 |
| Rendimiento sin Docker, programas oficiales (5 algoritmos) | `nativo_oficial/rendimiento.csv` | 3 400 (`fig5` 1 350, `afinidad` 1 350, `pares` 700) |
| Latencia sin Docker, programas oficiales | `nativo_oficial/latencia.csv` | 1 260 (`fig6` 675, `latafin` 585) |
| Memoria retenida sin Docker | `nativo_oficial/basura.csv` | 75 |
| Receta exacta del paper sin Docker (10 s × 7) | `nativo_oficial/paridad.csv` | 30 |
| Rendimiento sin Docker (`bench.cpp`, 4 algoritmos) | `nativo/rendimiento.csv` | 2 328 |
| Latencia sin Docker (`bench.cpp`) | `nativo/latencia.csv` | 648 |
| Rendimiento sin Docker (`bench.cpp` con las pilas alineadas) | `nativo_alineado/rendimiento.csv` | 2 160 (`fig5` 1 080, `afinidad` 1 080) |
| Latencia con las pilas alineadas | `nativo_alineado/latencia.csv` | 0: no se midió, solo tiene la cabecera |
| Controles de alineación y de Docker | `controles.csv` | 504 |
| Control de Williams con `std::atomic` y `boost::atomic` | `control_williams.csv` | 42 |
| Tráfico real de RAM | `memoria_ram.csv` | 70 |
| Tráfico real de RAM con y sin alinear las pilas (`bench.cpp`) | `nativo_alineado/memoria_alineado.csv` | 48 |

Todas las ejecuciones terminaron con código 0. `nativo_oficial/` y `nativo_alineado/` se midieron con `bash replica/completa/pendientes.sh` la noche del 6 al 7 de noviembre de 2026; `python3 replica/completa/herramientas/comprobar_final.py` confirma que tienen todas las filas esperadas.

## Columnas de los CSV

**`rendimiento.csv`:** `experimento, motor, impl, n, u, hilos, colocacion, familia, ronda, mops, rss_kb, segundos_reales, carga1, temp_c, codigo`

| Columna | Significado |
|---|---|
| `experimento` | `fig5` (rendimiento), `afinidad` (hilos fijados a núcleos) o `pares` (2 hilos en un par de núcleos concreto) |
| `motor` | `oficial` (binario del paper, en `docker/` y `nativo_oficial/`) o `equipo` (`codigo/bench.cpp`, en `nativo/` y `nativo_alineado/`) |
| `familia` | Grupo de la colocación: `sistema` en `fig5`; `E-compacto`, `E-disperso`, `P`, etc. en `afinidad`; y en `pares`, la clase del par: `mismo clúster`, `clústeres distintos`, `P ↔ P` o `P ↔ E` |
| `ronda` | Número de repetición. En `nativo_oficial/`, los pares tienen 10 rondas y la latencia `fig6`, 15 |
| `codigo` | Código de salida del programa; 0 es que terminó bien |
| `n`, `u` | Número de pilas y porcentaje de operaciones que escriben |
| `colocacion` | `sistema` (lo decide Linux), `E4-compacto`, `E4-disperso`, `P2`, `todos24`, `par-8-12`, etc. |
| `mops` | Millones de operaciones por segundo. Más es mejor |
| `rss_kb` | Pico de memoria física del proceso |
| `carga1`, `temp_c` | Carga de la máquina y temperatura en ese momento |

**`latencia.csv`:** `experimento, motor, impl, op, p, colocacion, familia, ronda, p1, p25, p50, p75, p99, p9995, iteraciones, carga1, temp_c, codigo`. Los percentiles están en **nanosegundos**. `op` es `load`, `store` o `store-del` (store y liberar). `p` es el número de hilos. `experimento` es `fig6` (sin fijar los hilos) o `latafin` (hilos fijados). En `nativo/` no hay `p9995`.

**`basura.csv`** (en `docker/` y `nativo_oficial/`): `ronda, impl, hilos, escritores, lectores, limite_objetos, pico_objetos, media_objetos, limite_bloques, pico_bloques, exceso_bloques, pico_bytes, operaciones`. Cuenta los objetos y bloques de memoria vivos; `limite_*` es el máximo si todo se liberara al instante (fórmula propia, no del paper).

**`paridad.csv`** (en `docker/` y `nativo_oficial/`): `impl, n, u, hilos, mediana_7x10s, minimo, maximo`. Mop/s con la receta exacta del paper: 7 ejecuciones de 10 s en un mismo proceso.

**`controles.csv`:** `control, variante, impl, n, u, hilos, ronda, mops`. `control` es `alineacion` (variantes `sin alinear` y `alineado`) o `docker` (variantes `fuera` y `dentro`).

**`control_williams.csv`:** `variante, par, n, u, hilos, ronda, mops`. `par` es la pareja de núcleos, o `sistema` si no se fijaron.

**`memoria_ram.csv`:** `prueba, impl, hilos, ronda, mops, gbs_lectura, gbs_escritura, bytes_por_op`. `prueba` es `reposo`, `lectura`, `copia`, `azar` (el techo de la máquina) o `pila` (los algoritmos con n = 10⁶ y u = 50 %). `bytes_por_op` no resta el tráfico en reposo; `tablas.md` sí lo resta.

**`nativo_alineado/memoria_alineado.csv`:** `impl, n, version, ronda, mops, gbs_lectura, gbs_escritura, bytes_por_op`. `version` es `sin_alinear` (`bin/bench_equipo`) o `alineado` (`bin/bench_equipo_alineado`). Siempre 24 hilos, u = 50 % y 10 s; `n` es 100 000 o 1 000 000. Las dos versiones de cada punto se midieron seguidas. Igual que en `memoria_ram.csv`, `bytes_por_op` no resta el reposo y `tablas.md` sí.

## Carpetas

| Carpeta o archivo | Contenido |
|---|---|
| `docker/` | CSV con Docker, más `basura.csv` (memoria retenida), `paridad.csv` (10 s × 7 frente a 2 s × 5) y los registros de progreso |
| `docker/latencia_williams_con_candado.csv` | Medidas de latencia de Williams **descartadas**: el binario se compiló sin `-mcx16` y llevaba un candado escondido. No usar. Se conservan como evidencia del error |
| `nativo_oficial/` | CSV sin Docker con los binarios oficiales: rendimiento, latencia, memoria retenida (`basura.csv`) y receta exacta (`paridad.csv`). Su registro está en `replica/completa/bin/pendientes.log`, que no se versiona |
| `nativo/` | CSV sin Docker con `bench.cpp` del equipo y su registro de progreso |
| `nativo_alineado/` | CSV sin Docker con una copia de `bench.cpp` que alinea las pilas (`alignas(128)`). Solo rendimiento: `latencia.csv` está vacío. `memoria_alineado.csv` es la prueba de la RAM que explica por qué alinear frena con n = 10⁶ |
| `correccion/` | Salidas de las pruebas de corrección: demo, ASan, TSan, boceto didáctico y variantes, la cota de intentos de `CAS` (`cota_cas.csv`) y los detectores sobre los rivales (`10_sanitizers_rivales.txt`). `latencia.csv` y `rendimiento.csv` de esta carpeta son una prueba rápida de `bench.cpp` y no se usan en las comparaciones |
| `topologia/matriz_nucleos.csv` | Latencia en ns entre cada par de núcleos (`a`, `b`, `ns`) |
| `figuras/` | Todas las gráficas en PNG |
| `tablas.md` | Todas las tablas, generadas por `analizar.py` |
| `equipo.txt` | Descripción de la máquina y las versiones de compilador |

## Figuras

| Figura | Qué muestra |
|---|---|
| `fig5_replica.png` | Rendimiento de los 5 algoritmos con Docker, 6 escenarios |
| `fig5_paper_frente_a_replica.png` | El paper y la réplica, escenario por escenario |
| `fig6_replica.png` | Latencia de `load`, `store` y `store & reclaim`, con p = 1 y p = 24 |
| `memoria_retenida.png` | Pico de bloques de memoria vivos según el número de hilos, frente al límite |
| `memoria_ram.png` | Tráfico real de RAM y bytes por operación con n = 10⁶, frente al techo de la máquina |
| `topologia.png` | Mapa de latencias entre núcleos |
| `afinidad_dosis_respuesta.png` | Con 4 hilos: del mismo clúster a clústeres distintos y a núcleos P |
| `afinidad_mapa_4hilos.png`, `afinidad_mapa_8hilos.png` | Cambio de rendimiento (%) al separar los hilos, por algoritmo y escenario |
| `miniatura_frente_a_paper.png` | Paper y réplica según el porcentaje de núcleos en uso |
| `nativo_fig5.png`, `nativo_fig6.png` | Rendimiento y latencia sin Docker, con `bench.cpp` |
| `nativo_dosis_respuesta.png` | La dosis-respuesta de 4 hilos sin Docker, con `bench.cpp` |
| `docker_frente_a_nativo.png` | Cada punto de la figura 5: programa oficial con Docker contra `bench.cpp` sin Docker |
| `nativo_oficial_fig5.png`, `nativo_oficial_fig6.png` | Rendimiento y latencia sin Docker, con los programas oficiales |
| `nativo_oficial_dosis_respuesta.png` | La dosis-respuesta de 4 hilos sin Docker, con los programas oficiales |
| `nativo_oficial_memoria_retenida.png` | Memoria retenida sin Docker, con los programas oficiales |
| `docker_frente_a_nativo_oficial.png` | Cada punto de la figura 5 con los programas oficiales, con Docker contra sin Docker |
| `nativo_alineado_fig5.png` | Rendimiento sin Docker con `bench.cpp` y las pilas alineadas |

## Cuidado al interpretar

- **`nativo/` y `nativo_alineado/` usan otro programa** (`bench.cpp`), no el del paper. No se pueden restar de `docker/` sin leer antes la Parte 6.6 de EXPLICACION.md. Los paneles n = 10 de `nativo/` están frenados por compartición falsa; compara con `nativo_alineado/`. La comparación limpia con Docker es `nativo_oficial/`, que usa los mismos programas oficiales.
- **`docker/` y `nativo_oficial/` no solo se diferencian en Docker:** cambian también g++ (12.3 y 12.4), glibc (2.35 y 2.39), Boost (1.74 y 1.83), jemalloc (5.2.1 y 5.3.0) y la forma de compilar Folly.
- **Temperatura:** en `nativo_oficial/rendimiento.csv`, 17 ejecuciones llegaron a 90 °C o más (máximo 98 °C); en `nativo_alineado/`, 1 (máximo 93 °C). Están repartidas y se conservan.
- **Los percentiles de latencia con p = 24 y p = 56 son muy variables** entre rondas: con Docker y p = 24, la ronda más alta suele triplicar a la más baja; sin Docker y con 15 rondas, casi la sextuplica. Una mediana aislada puede engañar; mira las tablas de variabilidad de `tablas.md`.
- **En `memoria_ram.csv`, los valores de 1 hilo son orientativos:** sus dos rondas difieren hasta un 41 %, porque el contador avanza a saltos de 1 GB. Con 12 hilos o más difieren un 6 % como mucho.
- **Esta CPU no es la del paper.** Compara el orden y las proporciones entre algoritmos, no los valores absolutos.
