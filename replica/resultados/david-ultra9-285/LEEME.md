# Resultados de david-ultra9-285

Máquina: Intel Core Ultra 9 285, 24 núcleos (8 P y 16 E en 4 clústeres), sin SMT, 1 nodo NUMA. Los detalles están en [equipo.txt](equipo.txt).

Para entender qué significan estos números, lee [../../completa/EXPLICACION.md](../../completa/EXPLICACION.md). Para regenerar las gráficas y tablas: `python3 replica/completa/analizar.py`.

## Resumen de lo que hay

| Qué | Dónde | Filas |
|---|---|---|
| Rendimiento con Docker (programas oficiales, 5 algoritmos) | `docker/rendimiento.csv` | 2 910 |
| Latencia con Docker | `docker/latencia.csv` | 810 |
| Rendimiento sin Docker (`bench.cpp`, 4 algoritmos) | `nativo/rendimiento.csv` | 2 328 |
| Latencia sin Docker | `nativo/latencia.csv` | 648 |
| Controles de alineación y de Docker | `controles.csv` | 504 |
| Control de Williams con `std::atomic` y `boost::atomic` | `control_williams.csv` | 42 |

Todas las ejecuciones terminaron con código 0. La carpeta `nativo_oficial/` **no existe todavía**: aparecerá al ejecutar `bash replica/completa/correr_nativo.sh oficial`.

## Columnas de los CSV

**`rendimiento.csv`:** `experimento, motor, impl, n, u, hilos, colocacion, familia, ronda, mops, rss_kb, segundos_reales, carga1, temp_c, codigo`

| Columna | Significado |
|---|---|
| `experimento` | `fig5` (rendimiento), `afinidad` (hilos fijados a núcleos) o `pares` (2 hilos en un par de núcleos concreto) |
| `motor` | `oficial` (binario del paper) o `equipo` (`codigo/bench.cpp`) |
| `n`, `u` | Número de pilas y porcentaje de operaciones que escriben |
| `colocacion` | `sistema` (lo decide Linux), `E4-compacto`, `E4-disperso`, `P2`, `todos24`, `par-8-12`, etc. |
| `mops` | Millones de operaciones por segundo. Más es mejor |
| `rss_kb` | Pico de memoria física del proceso |
| `carga1`, `temp_c` | Carga de la máquina y temperatura en ese momento |

**`latencia.csv`:** `experimento, motor, impl, op, p, colocacion, familia, ronda, p1, p25, p50, p75, p99, p9995, iteraciones, carga1, temp_c, codigo`. Los percentiles están en **nanosegundos**. `op` es `load`, `store` o `store-del` (store y liberar). `p` es el número de hilos. En `nativo/` no hay `p9995`.

**`controles.csv`:** `control, variante, impl, n, u, hilos, ronda, mops`. `control` es `alineacion` (variantes `sin alinear` y `alineado`) o `docker` (variantes `fuera` y `dentro`).

**`control_williams.csv`:** `variante, par, n, u, hilos, ronda, mops`. `par` es la pareja de núcleos, o `sistema` si no se fijaron.

## Carpetas

| Carpeta o archivo | Contenido |
|---|---|
| `docker/` | CSV con Docker, más `basura.csv` (memoria retenida), `paridad.csv` (10 s × 7 frente a 2 s × 5) y los registros de progreso |
| `docker/latencia_williams_con_candado.csv` | Medidas de latencia de Williams **descartadas**: el binario se compiló sin `-mcx16` y llevaba un candado escondido. No usar. Se conservan como evidencia del error |
| `nativo/` | CSV sin Docker y su registro de progreso |
| `correccion/` | Salidas de las pruebas de corrección: demo, ASan, TSan, boceto didáctico y variantes, y la cota de intentos de `CAS` (`cota_cas.csv`). `latencia.csv` y `rendimiento.csv` de esta carpeta son una prueba rápida de `bench.cpp` y no se usan en las comparaciones |
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
| `topologia.png` | Mapa de latencias entre núcleos |
| `afinidad_dosis_respuesta.png` | Con 4 hilos: del mismo clúster a clústeres distintos y a núcleos P |
| `afinidad_mapa_4hilos.png`, `afinidad_mapa_8hilos.png` | Cambio de rendimiento (%) al separar los hilos, por algoritmo y escenario |
| `miniatura_frente_a_paper.png` | Paper y réplica según el porcentaje de núcleos en uso |
| `nativo_fig5.png`, `nativo_fig6.png` | Rendimiento y latencia sin Docker |
| `nativo_dosis_respuesta.png` | La dosis-respuesta de 4 hilos sin Docker |
| `docker_frente_a_nativo.png` | Cada punto de la figura 5, con Docker contra sin Docker |

## Cuidado al interpretar

- **Con y sin Docker son programas distintos** (`bench_stack` contra `bench.cpp`). No se pueden restar uno del otro sin leer antes la Parte 6.6 de EXPLICACION.md. Los paneles n = 10 de `nativo/` están frenados por compartición falsa.
- **Los percentiles de latencia con p = 24 y p = 56 son muy variables** entre rondas. Una mediana aislada puede engañar; mira también el rango de las rondas.
- **Esta CPU no es la del paper.** Compara el orden y las proporciones entre algoritmos, no los valores absolutos.
