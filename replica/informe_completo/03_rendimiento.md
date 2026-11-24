# 03. Rendimiento, afinidad entre núcleos y controles (Figura 5)

Responsable original del apartado: Jheremy Manuel Alvarez Astete. Versión actualizada con la réplica completa en **Intel Core Ultra 9 285** (binarios oficiales de los 5 algoritmos dentro y fuera de Docker, validación de recortes, simulación de distancia entre clústeres y análisis de compartición falsa en `codigo/bench.cpp`).

## 1. Método experimental

Se evaluó el banco oficial `bench_stack` del artefacto de Zenodo (`10.5281/zenodo.11775182`, MD5 `e7e03425dfd426385a769788e5460fcd`) con `LD_PRELOAD` de `jemalloc` sobre pilas de Treiber concurrentes en los 6 escenarios $(n, u)$ de la Figura 5 del paper ($n \in \{1, 10, 10^6\}$ pilas; $u \in \{1, 10, 50\}\%$ de operaciones que extraen un nodo de una pila al azar y lo insertan en otra; el resto ejecuta `peek`/`load`).

- **Escalado de hilos:** la máquina del paper tenía 4 sockets Xeon E7-8867 (72 núcleos / 144 hilos, probados hasta 216 = $1,5\times$). En el Core Ultra 9 285 (24 núcleos sin SMT) se dividió la escala entre 6: `1, 3, 6, 9, 12, 18, 24, 30, 36` hilos, de modo que **24 hilos = 100 % de la CPU** y **36 hilos = 1,5×**.
- **Protocolo y validación:** cada punto se midió con la mediana de 5 ejecuciones de 2 s en procesos independientes y **orden aleatorio** (0 fallos en 2 910 ejecuciones con Docker y 3 400 sin Docker; dispersión típica CV = 1,0–1,1 %). Después se validó el recorte frente a la receta exacta del artefacto (`7 × 10 s` en un mismo proceso) en los 6 escenarios.
- **Simulación de distancia entre «sockets» (`fijar_hilos.c`):** tras medir con `latencia_nucleos.cpp` las latencias físicas caché-a-caché del Ultra 9 (`35,0 ns` dentro de un clúster E, `47,4 ns` = `1,35×` entre clústeres E, `67,4 ns` P↔E y `86,3 ns` = `2,47×` P↔P), se interceptó `pthread_create` sin tocar el binario oficial para comparar colocaciones `E-compacto` frente a `E-disperso` y 14 pares específicos de núcleos.

## 2. Resultados principales en el Core Ultra 9 285

### 2.1 Figura 5 a máquina llena: Paper (144 hilos) frente a Réplica (24 hilos, Mop/s)

Fuente: `tablas.md` (`docker/rendimiento.csv` y `nativo_oficial/rendimiento.csv`). Entre paréntesis, cociente respecto a Autores.

| Escenario $(n, u)$ | Tanda | Autores | Anderson | Folly | Williams | Libstdc++ | $\tau$ Kendall |
|---|---|---:|---:|---:|---:|---:|---:|
| $n = 1,\ u = 1\%$ | Paper (144 h) | 5,86 (1,00×) | **9,98 (1,70×)** | 0,31 (0,05×) | 0,75 (0,13×) | 0,11 (0,02×) | +0,60 |
| | Réplica Docker (24 h) | **19,81 (1,00×)** | 18,64 (0,94×) | 9,11 (0,46×) | 4,83 (0,24×) | 1,19 (0,06×) | |
| | Réplica Nativo (24 h) | **19,90 (1,00×)** | 18,11 (0,91×) | 8,94 (0,45×) | 4,96 (0,25×) | 1,08 (0,05×) | |
| $n = 1,\ u = 10\%$ | Paper (144 h) | 0,83 (1,00×) | **1,38 (1,66×)** | 0,05 (0,06×) | 0,09 (0,11×) | 0,04 (0,05×) | +0,60 |
| | Réplica Docker (24 h) | 4,43 (1,00×) | **8,81 (1,99×)** | 4,84 (1,09×) | 1,63 (0,37×) | 0,60 (0,14×) | |
| | Réplica Nativo (24 h) | 4,54 (1,00×) | **8,77 (1,93×)** | 4,64 (1,02×) | 1,65 (0,36×) | 0,60 (0,13×) | |
| $n = 1,\ u = 50\%$ | Paper (144 h) | 0,17 (1,00×) | **0,31 (1,82×)** | 0,01 (0,06×) | 0,02 (0,12×) | 0,01 (0,06×) | +0,40 |
| | Réplica Docker (24 h) | 1,07 (1,00×) | **2,00 (1,87×)** | 1,47 (1,37×) | 0,37 (0,35×) | 0,17 (0,16×) | |
| | Réplica Nativo (24 h) | 1,07 (1,00×) | **2,03 (1,90×)** | 1,50 (1,40×) | 0,38 (0,36×) | 0,19 (0,17×) | |
| $n = 10,\ u = 1\%$ | Paper (144 h) | **46,34 (1,00×)** | 30,45 (0,66×) | 3,12 (0,07×) | 6,57 (0,14×) | 1,09 (0,02×) | +0,80 |
| | Réplica Docker (24 h) | **121,90 (1,00×)** | 74,04 (0,61×) | 45,51 (0,37×) | 34,69 (0,28×) | 10,82 (0,09×) | |
| | Réplica Nativo (24 h) | **126,70 (1,00×)** | 72,31 (0,57×) | 46,84 (0,37×) | 35,55 (0,28×) | 10,95 (0,09×) | |
| $n = 10,\ u = 10\%$ | Paper (144 h) | 8,90 (1,00×) | **13,75 (1,54×)** | 0,53 (0,06×) | 1,04 (0,12×) | 0,35 (0,04×) | +0,80 |
| | Réplica Docker (24 h) | 55,40 (1,00×) | **62,67 (1,13×)** | 36,55 (0,66×) | 16,08 (0,29×) | 5,42 (0,10×) | |
| | Réplica Nativo (24 h) | 55,67 (1,00×) | **62,10 (1,12×)** | 36,78 (0,66×) | 16,09 (0,29×) | 5,24 (0,09×) | |
| $n = 10^6,\ u = 50\%$ | Paper (144 h) | **129,32 (1,00×)** | 77,50 (0,60×) | 51,41 (0,40×) | 76,12 (0,59×) | 112,68 (0,87×) | +0,20 |
| | Réplica Docker (24 h) | 55,37 (1,00×) | 56,50 (1,02×) | 53,39 (0,96×) | 40,39 (0,73×) | 56,34 (1,02×) | |
| | Réplica Nativo (24 h) | 54,66 (1,00×) | 56,21 (1,03×) | 54,12 (0,99×) | 41,28 (0,76×) | 54,78 (1,00×) | |

### 2.2 Hallazgos clave de rendimiento y controles metodológicos

1. **Lo que se reproduce con nitidez (afirmaciones 1, 2, 4 y 6):** Autores supera ampliamente a Libstdc++ bajo contención (`0,05×` a `0,17×`) y a Williams en los 6 escenarios (`0,24×` a `0,76×`). Con pocas actualizaciones ($u = 1\%$) supera a Anderson (`+65 %` en Docker y `+75 %` sin Docker en $n = 10$), mientras que con escrituras intensas sobre una pila ($n = 1, u \ge 10\%$) Anderson gana por `1,87×–1,99×` porque sus lecturas no modifican el puntero compartido ni invalidan los `CAS` de los escritores.
2. **Por qué Folly no se reproduce (afirmación 3):** en el paper Folly rinde veinte veces menos que Autores con contención (`0,05×–0,07×`); aquí gana 1 escenario ($n = 1, u = 50\%$: `1,47` vs `1,07 Mop/s`), empata 2 y pierde 3. La prueba decisiva aparece **con 1 solo hilo**, donde no hay coherencia entre núcleos: en el paper Folly ya caía a `0,18×–0,46×` de Autores, mientras que en el Ultra 9 rinde `0,86×–1,29×` (y Williams a 1 hilo sí coincide: `0,36×–0,68×` en el paper frente a `0,30×–0,70×` aquí). Se repite igual sin Docker y con la receta larga `7 × 10 s`.
3. **Docker no altera los binarios oficiales:** sobre los 270 puntos de la Figura 5 (6 paneles × 9 conteos de hilos × 5 algoritmos), **259 puntos cambian menos del 10 %** y **236 menos del 5 %** (medianas nativo/Docker entre `0,99×` y `1,01×`).
4. **Validación del recorte (`2 s × 5` frente a `10 s × 7`):** en los 30 puntos sin Docker, la desviación mediana absoluta es `< 6,7 %` (rango de `-10,0 %` a `+19,0 %`), manteniendo idéntico el orden en 4 de los 6 paneles y cruzando solo empates conocidos en los otros 2. Se observó un sesgo sistemático (el recorte da más con contención en 23/25 puntos y menos en los 5 puntos de $n = 10^6$), razón por la cual solo se declaran victorias ante diferencias amplias.
5. **Simulación de distancia entre clústeres:** al separar 4 hilos de un mismo clúster E (`35,0 ns`, L2 compartida) a 4 clústeres distintos (`47,4 ns`), Autores pierde entre `32 %` y `54 %` en los escenarios con contención, pero cambia apenas `±2 %` a `+3 %` en el control sin contención ($n = 10^6$). En las pruebas de 2 hilos sobre 14 pares (`n = 1, u = 50 %`), pasar de compartir L2 a no compartirla produce un escalón de `-44 %` a `-45 %` en Autores (`6,21` → `3,41–3,45 Mop/s`), mientras que duplicar la latencia hasta `86,3 ns` (P↔P) apenas añade cambio (`3,29–3,32 Mop/s`). Williams en `bench_stack` mostró una caída anómala entre clústeres (`-97 %` en Docker, `-76 %` sin Docker) que no ocurre en `bench.cpp` ni al intercambiar `std::atomic` por `boost::atomic`.
6. **Compartición falsa y caché L3 en `codigo/bench.cpp`:** en `bench.cpp` sin alinear, las 10 pilas de Autores ocupaban 80 bytes (~2 líneas de caché), frenándolo en $n = 10, u = 1\%$ a `35,35 Mop/s`. Al añadir `alignas(128)` en una copia (`nativo_alineado`), Autores subió **3,40×** hasta `120,27 Mop/s` (alineado con los `121,90–126,70 Mop/s` del binario oficial), sin alterar $n = 1$ (`1,00×–1,02×`). En cambio, con $n = 10^6$, `alignas(128)` redujo el rendimiento un `24–27 %` (`0,73×–0,76×`) porque el arreglo de pilas pasó de `8–16 MB` (cabe en los `36 MB` de L3) a `128 MB`, añadiendo `207–265 bytes` de tráfico de RAM por operación (`alinear_memoria.py`).
