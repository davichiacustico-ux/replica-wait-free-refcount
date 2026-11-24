# 04. Memoria, techo de RAM y contraste en un segundo equipo

Responsable original del apartado: Diego Quispe Candelario. Versión completa y actualizada con las mediciones de memoria retenida (`bench_basura`), RSS y tráfico de controladora de memoria (`medir_memoria.py`) en el **Intel Core Ultra 9 285**, más la comparativa experimental con el segundo equipo del estudio (**Intel Core i5-10400**).

## 1. Sobrecoste de memoria y reclamación diferida (Afirmación 13)

El paper afirma que los esquemas con liberación inmediata (Autores, Folly, Williams y Libstdc++) no tienen sobrecoste espacial, mientras que Anderson (basado en *hazard pointers* sobre bloques de control en ParlayLib) incurre en un sobrecoste pequeño por reclamación diferida. Como el paper no publica una gráfica de memoria, se diseñó `herramientas/bench_basura.cpp`: la mitad de los hilos hace `store` y la otra mitad `load`, registrando en todo instante los objetos y bloques de control vivos frente al máximo teórico sin retención (`limite_bloques = 122` para 24 hilos).

### 1.1 Pico de bloques de control vivos y memoria retenida (`tablas.md` y `basura.csv`)

| Hilos | Límite sin diferir | Autores | Anderson | Folly | Williams | Libstdc++ |
|---:|---:|---:|---:|---:|---:|---:|
| 2 | 12 | 9 | **4 013** *(4 026)* | 9 | 9 | 9 |
| 4 | 22 | 13 | **6 808** *(6 960)* | 14 | 15 | 13 |
| 8 | 42 | 21 *(22)* | **9 910** *(11 594)* | 23 | 25 | 21 |
| 16 | 82 | 36 *(37)* | **17 079** *(16 291)* | 39 *(40)* | 43 | 36 *(37)* |
| 24 | 122 | **52** *(54)* | **19 960** *(20 256)* | 53 *(54)* | 59 | 53 |

*(Valores en redonda: mediana en Docker; en cursiva entre paréntesis: sin Docker cuando difiere.)*

- **Qué retiene Anderson y cuánto ocupa:** el objeto de usuario se destruye en el acto en los cinco algoritmos (pico de 13 a 18 objetos vivos con 24 hilos, bajo el límite de 37). Lo que Anderson acumula en su lista de retiro son únicamente **bloques de control**: una mediana de `19 960` bloques (`469 KB`) en Docker y `20 256` bloques (`476 KB`) sin Docker (con picos máximos por ronda de hasta `509,6–516,2 KiB`), frente a `2,3 KB` de Autores (`0` bloques diferidos). Se confirma exactamente la afirmación 13 del paper.
- **Pico de RSS del proceso en $n = 10^6, u = 50\%, 24$ hilos (`tablas.md`):** cuando hay millones de nodos vivos, el RSS total lo dominan las pilas y no esos ~470 KB diferidos: Autores ocupa `1 074 MB`, Folly `1 055 MB`, Libstdc++ `1 055 MB`, Anderson `910 MB` y Williams `1 667 MB` (cuya celda atómica ocupa 16 bytes en lugar de 8).

## 2. Tráfico real de RAM en $n = 10^6$ (`medir_memoria.py`, Afirmación 5)

El Core Ultra 9 285 cuenta con **un solo módulo de 32 GB DDR5-5600** (canal único; máximo teórico `44,8 GB/s`). Leyendo los contadores hardware de la controladora integrada de memoria (`contar_imc.c` vía `perf_event_open`) junto con `ancho_banda.cpp`, el techo medido fue de **`42,7 GB/s` en lectura secuencial** y **`36,8 GB/s` (~37 GB/s) en accesos aleatorios**, que es el patrón de $n = 10^6$ pilas:

| Algoritmo (24 hilos, $n = 10^6, u = 50\%$) | Rendimiento (Mop/s) | Tráfico total IMC (GB/s) | Fracción del techo aleatorio (36,8 GB/s) | Bytes de RAM / op (sin reposo) |
|---|---:|---:|---:|---:|
| Libstdc++ | 55,3 | 34,1 | **92 %** | 603 |
| Anderson | 54,5 | 34,2 | **93 %** | 615 |
| Autores | 52,5 | 27,8 | 75 % | **515** |
| Folly | 52,4 | 32,8 | **89 %** | 613 |
| Williams | 38,6 | 29,9 | 81 % | 755 |

Tres algoritmos rozan el 89–93 % del ancho de banda aleatorio de este canal único, lo que explica en gran parte el empate en torno a `53–57 Mop/s` (frente a los `129,3 Mop/s` en los 4 sockets con memoria distribuida del paper). Aun así, Autores empata consumiendo solo el 75 % del techo y menos bytes por operación (`515 B`), por lo que la prueba definitiva requeriría añadir un segundo módulo DDR5 en doble canal.

## 3. Contraste entre dos equipos: Core Ultra 9 285 frente a Core i5-10400

Ambas máquinas ejecutaron exactamente el mismo programa y guion rápido (`codigo/ejecutar_pruebas.sh`, `bench.cpp` sin alinear, 1 s por punto, g++ 13.3):
- **Equipo A (Réplica principal):** Intel Core Ultra 9 285 (24 núcleos: 8 P + 16 E en 4 clústeres, sin SMT, 36 MB L3, 1×32 GB DDR5-5600 canal único, Linux nativo). Fuente: `resultados/david-ultra9-285/correccion/rendimiento.csv`.
- **Equipo B (Segundo equipo):** Intel Core i5-10400 (6 núcleos / 12 hilos con SMT, 12 MB L3, 24 GB DDR4-2667 en doble canal, WSL2 Ubuntu 24.04). Fuente: `capturas/rendimiento.csv`.

### 3.1 Comparación directa a 1 hilo y a 12 hilos (`bench.cpp`, Mop/s)

| Escenario | Algoritmo | 1 hilo: i5-10400 | 1 hilo: Ultra 9 | Ultra 9 / i5 (1 h) | 12 hilos: i5-10400 | 12 hilos: Ultra 9 | Ultra 9 / i5 (12 h) |
|---|---|---:|---:|---:|---:|---:|---:|
| $n = 1,\ u = 1\%$ | **Autores** | 44,83 | **96,94** | **2,16×** | **39,13** | 21,79 | **0,56×** |
| | Williams | 19,48 | 34,83 | 1,79× | 5,34 | 4,94 | 0,93× |
| | Libstdc++ | 33,53 | 55,65 | 1,66× | 1,58 | 1,56 | 0,99× |
| | Mutex | 27,72 | 47,89 | 1,73× | 6,07 | 7,61 | 1,25× |
| $n = 1,\ u = 50\%$ | **Autores** | 14,56 | **21,95** | 1,51× | **2,32** | 1,64 | **0,71×** |
| | Mutex | 10,65 | 18,43 | 1,73× | 1,51 | **1,82** | 1,21× |
| $n = 10,\ u = 1\%$ *(sin alinear)* | **Autores** | 48,38 | **95,49** | 1,97× | **50,61** | 32,20 | **0,64×** |
| $n = 10^5,\ u = 50\%$ | **Autores** | 4,67 | **14,31** | **3,07×** | 26,69 | **73,23** | **2,74×** |
| | Williams | 2,86 | 5,35 | 1,87× | 18,02 | 44,07 | 2,45× |
| | Libstdc++ | 4,02 | 11,89 | 2,96× | 18,37 | 70,54 | 3,84× |

### 3.2 Qué se mantiene y qué cambia entre arquitecturas

1. **Las conclusiones algorítmicas se mantienen en ambos equipos:** en el i5-10400, igual que en el Ultra 9, Autores elimina la carrera *read-reclaim* (0 errores ASan/TSan), supera con claridad a Williams (`7,3×` en $n=1, u=1\%$ a 12 hilos: `39,13` vs `5,34 Mop/s`, muy cerca del `7,8×` del paper) y multiplica por `24,8×` a Libstdc++ (`39,13` vs `1,58 Mop/s`), manteniendo además la cola de latencia `p99` más corta (`0,93 µs` frente a `6,06 µs` de Williams, `19,89 µs` de Libstdc++ y `25,83 µs` del Mutex).
2. **Inversión bajo contención extrema:** con 1 solo hilo o sin contención ($n = 10^5$), el Core Ultra 9 285 es entre **2,16× y 3,07× más rápido** que el Core i5-10400 gracias a su arquitectura más moderna y a sus 36 MB de L3. Sin embargo, cuando los 12 hilos golpean una misma línea de caché ($n = 1, u = 1\%$), **el i5-10400 supera al Ultra 9 (`39,13` frente a `21,79 Mop/s`, `0,56×`)**. La explicación plausible —marcada como hipótesis pendiente de aislar fijando hilos en el i5— es topológica: en el i5-10400 los 12 hilos conviven en un único anillo monolítico de 6 núcleos físicos donde cada pareja de hermanos SMT comparte L1 y L2, mientras que en el Ultra 9 los rebotes de línea entre clústeres E y núcleos P cuestan entre `47,4` y `86,3 ns`.
