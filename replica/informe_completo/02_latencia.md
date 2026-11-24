# 02. Latencia y comportamiento de cola (Figura 6)

Responsable original del apartado: David Tamo Turpo. Versión actualizada con los binarios oficiales del artefacto (los 5 algoritmos del paper, con y sin Docker, 5 y 15 rondas en el **Core Ultra 9 285**) y el contraste con `codigo/bench.cpp` en ambas máquinas (**Ultra 9 285** e **i5-10400**). Corrige además la errata del borrador anterior (`99,7 µs` para el `load` de Libstdc++ con $p = 56$ en la Figura 6 del paper).

## 1. Método experimental

El banco oficial (`bench_lat` del artefacto de Zenodo) lanza $p$ hilos: el hilo 0 cronometra repetidamente una operación (`load`, `store` sin liberar o `store & reclaim` bajando el contador a 0), mientras los $p - 1$ hilos restantes ejecutan `store` en bucle cerrado sobre la misma variable compartida.

- **Valores de $p$ en el Core Ultra 9 285 (24 núcleos sin SMT):** $p = 1$ (sin contención), $p = 24$ (toda la máquina ocupada sin sobresuscripción, equivalente funcional al $p = 56$ del Xeon w7-3465X del paper) y $p = 56$ (medido por completitud, pero con sobresuscripción de 2,33× en esta CPU).
- **Rondas:** 5 rondas con Docker (`docker/latencia.csv`) y **15 rondas** sin Docker (`nativo_oficial/latencia.csv`) para estabilizar la mediana frente a la alta variabilidad con contención.
- **Activación y corrección de Williams (`-mcx16`):** el paper omitió a Williams en la Figura 6 indicando que su latencia «casi siempre era mayor». Al descomentar su línea en el artefacto se descubrió que el `CMakeLists.txt` de latencia **no incluía `-mcx16`**, haciendo que `boost::atomic` de 16 bytes cayera silenciosamente en un candado interno (`lock_pool`, dando `40 680 ns` en `load` con $p = 24$). Se compiló un binario dedicado con `-mcx16` (`build_cx16/`, verificado con `cmpxchg16b` y 0 llamadas a `lock_pool`); los datos con candado quedaron apartados en `latencia_williams_con_candado.csv` y no se usan en las comparaciones.
- **Segunda opinión (`codigo/bench.cpp latencia`):** medida en el Core i5-10400 ($p = 1$ y $p = 12$) y en el Core Ultra 9 285 ($p = 1, 24, 56$), incluyendo un `std::mutex` como referencia bloqueante adicional.

## 2. Resultados con los binarios oficiales (Core Ultra 9 285)

### 2.1 Medianas (`p50`) en nanosegundos: Paper frente a Réplica (con Docker y sin Docker, 15 rondas)

Fuente: `tablas.md` (secciones «Latencia: medianas en ns (figura 6)» y «Latencia: medianas en ns (nativo, binarios oficiales)»).

| Operación | Configuración | Autores | Anderson | Folly | Williams (`-mcx16`) | Libstdc++ |
|---|---|---:|---:|---:|---:|---:|
| `load` | Paper ($p = 1$) | **23** | 28 | 24 | — | 29 |
| | Ultra 9 Docker ($p = 1$, 5 rondas) | **14** | 18 | 15 | 34 | 20 |
| | Ultra 9 Nativo ($p = 1$, 15 rondas) | **14** | 18 | 15 | 34 | 20 |
| `load` | Paper ($p = 56$, CPU llena) | **1 140** | 2 580 | 11 600 | — | 99 700 |
| | Ultra 9 Docker ($p = 24$, CPU llena) | **437** | 488 | 473 | 3 104 | 3 716 |
| | Ultra 9 Nativo ($p = 24$, 15 rondas) | **414** | 511 | 465 | 6 106 | 3 882 |
| | Ultra 9 Nativo ($p = 56$, sobresuscrito) | **1 473** | 1 572 | 3 281 | 22 150 | 40 463 |
| `store` | Paper ($p = 1$) | 33 | **28** | 37 | — | 36 |
| | Ultra 9 Docker / Nativo ($p = 1$) | 22 / 22 | 18 / 18 | **17 / 17** | 32 / 32 | 20 / 20 |
| `store` | Paper ($p = 56$, CPU llena) | **1 470** | 2 840 | 2 620 | — | 111 000 |
| | Ultra 9 Docker ($p = 24$) | 507 | **483** | 832 | 1 021 | 3 411 |
| | Ultra 9 Nativo ($p = 24$, 15 rondas) | **461** | 470 | 794 | 756 | 2 746 |
| `store & reclaim` | Paper ($p = 1$) | 44 | 44 | 49 | — | **41** |
| | Ultra 9 Docker / Nativo ($p = 1$) | 28 / 27 | 29 / 29 | **23 / 24** | 44 / 45 | 25 / 25 |
| `store & reclaim` | Paper ($p = 56$, CPU llena) | 1 570 | **1 250** | 5 060 | — | 59 300 |
| | Ultra 9 Docker ($p = 24$) | **462** | 470 | 1 521 | 821 | 3 248 |
| | Ultra 9 Nativo ($p = 24$, 15 rondas) | **456** | 468 | 1 290 | 1 116 | 3 002 |

### 2.2 Colas de latencia (`p50 / p99 / p99,95` en ns, sin Docker, 15 rondas)

| Operación y $p$ | Autores | Anderson | Folly | Williams | Libstdc++ |
|---|---|---|---|---|---|
| `load`, $p = 1$ | 14 / 15 / 16 | 18 / 19 / 20 | 15 / 17 / 23 | 34 / 36 / 38 | 20 / 21 / 27 |
| `load`, $p = 24$ | **414 / 848 / 1 112** | 511 / 1 014 / 1 823 | 465 / 2 091 / 4 006 | 6 106 / 21 670 / 32 822 | 3 882 / 48 062 / 90 014 |
| `store & reclaim`, $p = 1$ | 27 / 29 / **30** | 29 / 32 / **7 740** | 24 / 26 / 31 | 45 / 48 / 51 | 25 / 27 / 38 |
| `store & reclaim`, $p = 24$ | **456 / 887 / 1 308** | 468 / 900 / 1 295 | 1 290 / 2 585 / 3 246 | 1 116 / 2 479 / 3 143 | 3 002 / 40 882 / 76 730 |

*(Con Docker el salto de Anderson en `store & reclaim`, $p = 1$, `p99,95` es idéntico: `29 / 32 / 7 895 ns`.)*

### 2.3 Segunda opinión con `bench.cpp` en ambos equipos (`load` con la CPU llena)

| Equipo (`bench.cpp`) | $p$ | Autores `p50` (`p99`) | Williams `p50` (`p99`) | Libstdc++ `p50` (`p99`) | Mutex `p50` (`p99`) | Fuente |
|---|---:|---|---|---|---|---|
| **Core i5-10400** (1 pasada) | 12 | 307,9 ns (**929,4 ns**) | 1 638,4 ns (6 059,9 ns) | 3 189,4 ns (19 891,5 ns) | **118,5 ns** (25 828,8 ns) | `capturas/latencia.csv` |
| **Core Ultra 9 285** (pasada rápida) | 24 | 426,3 ns (**1 320,5 ns**) | 5 273,3 ns (22 631,9 ns) | 2 315,8 ns (34 836,0 ns) | **198,3 ns** (20 738,4 ns) | `correccion/latencia.csv` |

## 3. Análisis y contraste con las afirmaciones del paper

1. **`load` más bajo (afirmación 7 — Se reproduce, con poco margen bajo contención):** Autores logra la menor mediana en todos los cortes (`14 ns` con $p = 1$; `437` y `414 ns` con $p = 24$), pero bajo contención sus rondas se solapan con Anderson y Folly y exhiben comportamiento bimodal (`~400–445 ns` en 13 de 15 rondas sin Docker frente a `1 265–1 595 ns` en 2 rondas; al fijar los hilos con `todos24` el segundo modo no apareció en las 3 rondas medidas).
2. **`store` sin contención (afirmación 8 — No se reproduce) y con contención (afirmación 9 — En parte):** con $p = 1$, Autores queda cuarto de cinco (`22 ns` frente a `17 ns` de Folly, `18 ns` de Anderson y `20 ns` de Libstdc++). Con $p = 24$, empata en cabeza con Anderson (`507` vs `483 ns` en Docker; `461` vs `470 ns` sin Docker).
3. **Sobrecoste de liberar (afirmaciones 10 y 11):** con $p = 1$, `store & reclaim` es más lento que `store` en los cinco algoritmos (**Se reproduce**). En cambio, la mejora bajo contención (**afirmación 11**) que parecía verse con 5 rondas en Docker desaparece al subir a 15 rondas sin Docker (`461` → `456 ns` en $p = 24$ y `1 531` → `1 480 ns` en $p = 56$, variaciones del 1–4 % indistinguibles del ruido), por lo que queda en **No se confirma**.
4. **Omisión de Williams (afirmación 12 — En parte):** una vez compilado correctamente con `-mcx16`, Williams es efectivamente el más lento con $p = 1$ (`32–45 ns`), pero con contención supera a Libstdc++ en casi todos los casos (6 de 6 en Docker, 5 de 6 sin Docker) y a Folly en `store` y `store & reclaim` con $p = 24$.
5. **Lo que revelan las colas:** en `store & reclaim` con $p = 1$, Anderson salta de `32 ns` en `p99` a `7 740–7 895 ns` exactamente en `p99,95` (1 de cada 2 000 operaciones), coherente con el barrido periódico de su lista de retiro cada 2 000 bloques. Frente al `Mutex` en `bench.cpp`, aunque dormir hilos da mejor mediana aparente (`118,5 ns` en el i5), su cola `p99` se dispara a `25,8 µs` (`28×` peor que los `0,93 µs` de Autores).
6. **Límite metodológico:** con $p = 24$ y sin aislar núcleos ni fijar frecuencias, la razón entre la peor y la mejor ronda alcanza una mediana de `3,40×` (5 rondas Docker) y `5,75×` (15 rondas sin Docker). Por ello, diferencias pequeñas con contención deben leerse como empates.
