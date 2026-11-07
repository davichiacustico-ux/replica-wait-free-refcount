# Tablas generadas por analizar.py

Datos: `david-ultra9-285`.

## Latencia entre núcleos (mediana por clase)

| Clase | ns | Respecto a mismo clúster |
|---|---|---|
| E, mismo clúster | 35.0 | 1.00× |
| E, clústeres distintos | 47.4 | 1.35× |
| P ↔ E | 67.4 | 1.93× |
| P ↔ P | 86.3 | 2.47× |

## Rendimiento: paper (144 hilos) frente a réplica (24 hilos)

Mop/s y, entre paréntesis, relación con Autores. τ = acuerdo de orden entre paper y réplica (1 = idéntico).

| Panel | | Autores | Anderson | Folly | Williams | Libstdc++ | τ |
|---|---|---|---|---|---|---|---|
| n = 1, u = 1 % | paper | 5.86 (1.00×) | 9.98 (1.70×) | 0.31 (0.05×) | 0.75 (0.13×) | 0.11 (0.02×) |  |
|  | réplica | 19.81 (1.00×) | 18.64 (0.94×) | 9.11 (0.46×) | 4.83 (0.24×) | 1.19 (0.06×) | +0.60 |
| n = 1, u = 10 % | paper | 0.83 (1.00×) | 1.38 (1.66×) | 0.05 (0.06×) | 0.09 (0.11×) | 0.04 (0.05×) |  |
|  | réplica | 4.43 (1.00×) | 8.81 (1.99×) | 4.84 (1.09×) | 1.63 (0.37×) | 0.60 (0.14×) | +0.60 |
| n = 1, u = 50 % | paper | 0.17 (1.00×) | 0.31 (1.82×) | 0.01 (0.06×) | 0.02 (0.12×) | 0.01 (0.06×) |  |
|  | réplica | 1.07 (1.00×) | 2.00 (1.87×) | 1.47 (1.37×) | 0.37 (0.35×) | 0.17 (0.16×) | +0.40 |
| n = 10, u = 1 % | paper | 46.34 (1.00×) | 30.45 (0.66×) | 3.12 (0.07×) | 6.57 (0.14×) | 1.09 (0.02×) |  |
|  | réplica | 121.90 (1.00×) | 74.04 (0.61×) | 45.51 (0.37×) | 34.69 (0.28×) | 10.82 (0.09×) | +0.80 |
| n = 10, u = 10 % | paper | 8.90 (1.00×) | 13.75 (1.54×) | 0.53 (0.06×) | 1.04 (0.12×) | 0.35 (0.04×) |  |
|  | réplica | 55.40 (1.00×) | 62.67 (1.13×) | 36.55 (0.66×) | 16.08 (0.29×) | 5.42 (0.10×) | +0.80 |
| n = 10⁶, u = 50 % | paper | 129.32 (1.00×) | 77.50 (0.60×) | 51.41 (0.40×) | 76.12 (0.59×) | 112.68 (0.87×) |  |
|  | réplica | 55.37 (1.00×) | 56.50 (1.02×) | 53.39 (0.96×) | 40.39 (0.73×) | 56.34 (1.02×) | +0.20 |

## Relación con Autores con UN solo hilo

Rendimiento del rival dividido entre el de Autores.

| Panel | | Anderson | Folly | Williams | Libstdc++ |
|---|---|---|---|---|---|
| n = 1, u = 1 % | paper | 0.59× | 0.20× | 0.36× | 0.77× |
|  | réplica | 0.85× | 0.86× | 0.30× | 0.62× |
| n = 1, u = 10 % | paper | 0.49× | 0.19× | 0.40× | 0.65× |
|  | réplica | 1.03× | 1.03× | 0.36× | 0.79× |
| n = 1, u = 50 % | paper | 0.41× | 0.18× | 0.36× | 0.60× |
|  | réplica | 1.25× | 1.25× | 0.43× | 1.02× |
| n = 10, u = 1 % | paper | 0.60× | 0.20× | 0.42× | 0.72× |
|  | réplica | 0.87× | 0.87× | 0.30× | 0.62× |
| n = 10, u = 10 % | paper | 0.50× | 0.21× | 0.38× | 0.66× |
|  | réplica | 1.04× | 1.04× | 0.36× | 0.78× |
| n = 10⁶, u = 50 % | paper | 1.03× | 0.46× | 0.68× | 0.87× |
|  | réplica | 1.46× | 1.29× | 0.70× | 1.44× |

## Pico de memoria del proceso (RSS, MB), n = 10⁶, u = 50 %, 24 hilos

| Autores | Anderson | Folly | Williams | Libstdc++ |
|---|---|---|---|---|
| 1,074 | 910 | 1,055 | 1,667 | 1,055 |

## Validación de los recortes: receta exacta del paper (10 s × 7) frente a 2 s × 5

| Punto | Impl | 10 s × 7 | 2 s × 5 | Diferencia |
|---|---|---|---|---|
| n = 1, u = 1 % | Autores | 19.00 | 19.81 | +4.3 % |
| n = 1, u = 1 % | Libstdc++ | 1.39 | 1.19 | -14.8 % |
| n = 1, u = 1 % | Williams | 4.29 | 4.83 | +12.6 % |
| n = 1, u = 1 % | Folly | 7.53 | 9.11 | +21.0 % |
| n = 1, u = 1 % | Anderson | 16.99 | 18.64 | +9.7 % |
| n = 10⁶, u = 50 % | Autores | 56.30 | 55.37 | -1.7 % |
| n = 10⁶, u = 50 % | Libstdc++ | 60.43 | 56.34 | -6.8 % |
| n = 10⁶, u = 50 % | Williams | 42.44 | 40.39 | -4.8 % |
| n = 10⁶, u = 50 % | Folly | 57.70 | 53.39 | -7.5 % |
| n = 10⁶, u = 50 % | Anderson | 58.69 | 56.50 | -3.7 % |

## Calidad de las medidas (Docker)

- Ejecuciones: 2910; fallidas: 0.
- Temperatura: mediana 74 °C, máxima 95 °C.
- Dispersión entre rondas (coef. de variación): mediana 1.0 %, percentil 90 8.3 %.

## Efecto de separar los hilos entre «sockets» (Docker, binarios oficiales)

Cambio de rendimiento de `E-compacto` a `E-disperso`, en %. «≈» = los rangos de las rondas se solapan: no se distingue del ruido.


**2 hilos**

| Panel | Autores | Anderson | Folly | Williams | Libstdc++ |
|---|---|---|---|---|---|
| n = 1, u = 1 % | -53 % | -14 % | -27 % | -21 % | -43 % |
| n = 1, u = 10 % | -48 % | -27 % | -25 % | -88 % | -37 % |
| n = 1, u = 50 % | -45 % | -45 % | -28 % | -99 % | -36 % |
| n = 10, u = 1 % | -46 % | -23 % | -45 % | -35 % | -46 % |
| n = 10, u = 10 % | -43 % | -36 % | -42 % | -64 % | -41 % |
| n = 10⁶, u = 50 % | +1 % | ≈-0 % | +1 % | +1 % | ≈+0 % |

**4 hilos**

| Panel | Autores | Anderson | Folly | Williams | Libstdc++ |
|---|---|---|---|---|---|
| n = 1, u = 1 % | -32 % | ≈+0 % | +48 % | +25 % | -43 % |
| n = 1, u = 10 % | -49 % | -33 % | +11 % | -12 % | -41 % |
| n = 1, u = 50 % | -48 % | -50 % | -9 % | -37 % | ≈-12 % |
| n = 10, u = 1 % | -54 % | -31 % | -49 % | -34 % | -55 % |
| n = 10, u = 10 % | -50 % | -39 % | -44 % | -34 % | -50 % |
| n = 10⁶, u = 50 % | ≈+2 % | +2 % | +2 % | +2 % | ≈-2 % |

**8 hilos**

| Panel | Autores | Anderson | Folly | Williams | Libstdc++ |
|---|---|---|---|---|---|
| n = 1, u = 1 % | -6 % | ≈+0 % | ≈+72 % | ≈-27 % | ≈+31 % |
| n = 1, u = 10 % | +4 % | ≈-2 % | ≈+7 % | +90 % | ≈+20 % |
| n = 1, u = 50 % | ≈-0 % | -4 % | ≈+45 % | +105 % | ≈+5 % |
| n = 10, u = 1 % | -16 % | +1 % | -8 % | +11 % | -12 % |
| n = 10, u = 10 % | -13 % | ≈-2 % | +36 % | ≈+2 % | +3 % |
| n = 10⁶, u = 50 % | +1 % | +2 % | +1 % | ≈+0 % | ≈+5 % |

**12 hilos**

| Panel | Autores | Anderson | Folly | Williams | Libstdc++ |
|---|---|---|---|---|---|
| n = 1, u = 1 % | ≈+8 % | +1 % | ≈+37 % | ≈-10 % | +5 % |
| n = 1, u = 10 % | -1 % | -3 % | ≈+1 % | ≈+33 % | ≈-13 % |
| n = 1, u = 50 % | ≈-9 % | ≈+2 % | ≈+71 % | ≈-5 % | ≈+18 % |
| n = 10, u = 1 % | -4 % | +2 % | +28 % | +15 % | ≈+2 % |
| n = 10, u = 10 % | -5 % | ≈+1 % | +1 % | +18 % | ≈-2 % |
| n = 10⁶, u = 50 % | +0 % | +1 % | ≈+0 % | ≈+0 % | ≈-2 % |

## Dos hilos en distintos pares de núcleos (Docker, binarios oficiales)

Mop/s, mediana de 3 rondas (entre paréntesis, mínimo y máximo de Williams), n = 1, u = 50 %.

| Clase | Núcleos | Autores | Anderson | Folly | Williams | Libstdc++ |
|---|---|---|---|---|---|---|
| mismo clúster | 12 y 13 | 6.21 | 11.82 | 6.77 | 3.05 (3.05–3.06) | 5.07 |
| mismo clúster | 16 y 17 | 6.21 | 11.72 | 6.75 | 3.04 (3.03–3.05) | 5.07 |
| mismo clúster | 20 y 21 | 6.20 | 11.98 | 6.74 | 3.05 (3.05–3.05) | 5.07 |
| mismo clúster | 8 y 9 | 6.22 | 11.83 | 6.70 | 3.06 (3.05–3.06) | 5.06 |
| clústeres distintos | 12 y 16 | 3.41 | 6.48 | 4.75 | 0.22 (0.05–0.60) | 2.91 |
| clústeres distintos | 12 y 20 | 3.41 | 6.67 | 4.68 | 0.10 (0.04–0.18) | 3.20 |
| clústeres distintos | 16 y 20 | 3.36 | 6.44 | 4.84 | 0.04 (0.02–0.09) | 3.13 |
| clústeres distintos | 8 y 12 | 3.41 | 6.77 | 4.69 | 0.23 (0.04–0.26) | 2.73 |
| clústeres distintos | 8 y 16 | 3.41 | 6.50 | 4.31 | 0.12 (0.05–0.16) | 2.91 |
| clústeres distintos | 8 y 20 | 3.40 | 6.55 | 4.65 | 0.05 (0.04–0.91) | 3.10 |
| clústeres distintos | 9 y 13 | 3.38 | 6.54 | 4.63 | 0.03 (0.03–1.09) | 2.98 |
| P ↔ P | 2 y 3 | 3.31 | 6.41 | 4.29 | 1.76 (1.48–1.78) | 3.57 |
| P ↔ P | 4 y 5 | 3.27 | 6.34 | 4.42 | 1.55 (1.51–1.82) | 3.53 |
| P ↔ E | 2 y 12 | 3.41 | 6.52 | 4.40 | 1.65 (1.59–1.67) | 3.03 |

## Latencia: medianas en ns (figura 6)

| Operación | p | Autores | Anderson | Folly | Williams | Libstdc++ |
|---|---|---|---|---|---|---|
| load | paper p=1 | 23 | 28 | 24 | — | 29 |
| | réplica p=1 | 14 | 18 | 15 | 34 | 20 |
| load | paper p=56 | 1,140 | 2,580 | 11,600 | — | 99,700 |
| | réplica p=24 | 437 | 488 | 473 | 3,104 | 3,716 |
| load | paper p=56 | 1,140 | 2,580 | 11,600 | — | 99,700 |
| | réplica p=56 | 1,502 | 1,540 | 2,977 | 23,738 | 61,687 |
| store | paper p=1 | 33 | 28 | 37 | — | 36 |
| | réplica p=1 | 22 | 18 | 17 | 32 | 20 |
| store | paper p=56 | 1,470 | 2,840 | 2,620 | — | 111,000 |
| | réplica p=24 | 507 | 483 | 832 | 1,021 | 3,411 |
| store | paper p=56 | 1,470 | 2,840 | 2,620 | — | 111,000 |
| | réplica p=56 | 1,516 | 1,498 | 4,813 | 4,105 | 52,832 |
| store & reclaim | paper p=1 | 44 | 44 | 49 | — | 41 |
| | réplica p=1 | 28 | 29 | 23 | 44 | 25 |
| store & reclaim | paper p=56 | 1,570 | 1,250 | 5,060 | — | 59,300 |
| | réplica p=24 | 462 | 470 | 1,521 | 821 | 3,248 |
| store & reclaim | paper p=56 | 1,570 | 1,250 | 5,060 | — | 59,300 |
| | réplica p=56 | 813 | 1,499 | 6,525 | 4,439 | 47,198 |

## Colas de latencia en ns: p50 / p99 / p99,95

| Operación | p | Autores | Anderson | Folly | Williams | Libstdc++ |
|---|---|---|---|---|---|---|
| load | 1 | 14 / 15 / 16 | 18 / 19 / 20 | 15 / 16 / 22 | 34 / 36 / 38 | 20 / 21 / 22 |
| load | 24 | 437 / 1,175 / 1,634 | 488 / 952 / 1,408 | 473 / 3,008 / 5,566 | 3,104 / 7,251 / 10,019 | 3,716 / 34,847 / 65,230 |
| store | 1 | 22 / 23 / 24 | 18 / 19 / 20 | 17 / 18 / 23 | 32 / 35 / 37 | 20 / 22 / 22 |
| store | 24 | 507 / 1,321 / 1,735 | 483 / 926 / 1,167 | 832 / 2,056 / 2,732 | 1,021 / 16,593 / 27,387 | 3,411 / 38,851 / 71,939 |
| store & reclaim | 1 | 28 / 29 / 30 | 29 / 32 / 7,895 | 23 / 26 / 36 | 44 / 49 / 52 | 25 / 27 / 34 |
| store & reclaim | 24 | 462 / 881 / 1,174 | 470 / 894 / 1,199 | 1,521 / 2,559 / 3,180 | 821 / 1,977 / 2,790 | 3,248 / 31,386 / 59,355 |

## Memoria retenida: pico de bloques vivos

| Hilos | Límite | Autores | Anderson | Folly | Williams | Libstdc++ |
|---|---|---|---|---|---|---|
| 2 | 12 | 9 | 4,013 | 9 | 9 | 9 |
| 4 | 22 | 13 | 6,808 | 14 | 15 | 13 |
| 8 | 42 | 21 | 9,910 | 23 | 25 | 21 |
| 16 | 82 | 36 | 17,079 | 39 | 43 | 36 |
| 24 | 122 | 52 | 19,960 | 53 | 59 | 53 |

## Calidad de las medidas (Nativo)

- Ejecuciones: 2328; fallidas: 0.
- Temperatura: mediana 73 °C, máxima 99 °C.
- Dispersión entre rondas (coef. de variación): mediana 1.6 %, percentil 90 6.0 %.

## Docker frente a nativo (bench.cpp del equipo)

Cociente nativo/Docker de las medianas, sobre los 54 puntos de la figura 5 (6 paneles × 9 hilos).

| Impl | Mediana | Mínimo | Máximo |
|---|---|---|---|
| Autores | 1.06× | 0.25× | 1.84× |
| Libstdc++ | 0.71× | 0.44× | 1.49× |
| Williams | 0.55× | 0.32× | 1.27× |

## Dos hilos en distintos pares de núcleos (nativo, bench.cpp del equipo)

Mop/s, mediana de 3 rondas (entre paréntesis, mínimo y máximo de Williams), n = 1, u = 50 %.

| Clase | Núcleos | Autores | Libstdc++ | Williams | Mutex |
|---|---|---|---|---|---|
| mismo clúster | 12 y 13 | 8.84 | 4.36 | 2.02 (2.02–2.02) | 3.91 |
| mismo clúster | 16 y 17 | 8.83 | 4.28 | 2.02 (2.02–2.11) | 3.93 |
| mismo clúster | 20 y 21 | 8.82 | 4.34 | 2.02 (2.02–2.02) | 3.89 |
| mismo clúster | 8 y 9 | 8.82 | 4.29 | 2.02 (2.02–2.02) | 3.94 |
| clústeres distintos | 12 y 16 | 4.70 | 2.47 | 1.16 (1.14–1.17) | 3.72 |
| clústeres distintos | 12 y 20 | 4.44 | 2.46 | 1.15 (1.15–1.30) | 4.02 |
| clústeres distintos | 16 y 20 | 4.41 | 2.58 | 1.17 (1.15–1.21) | 3.95 |
| clústeres distintos | 8 y 12 | 4.40 | 2.66 | 1.17 (1.11–1.22) | 3.65 |
| clústeres distintos | 8 y 16 | 4.41 | 2.62 | 1.14 (1.14–1.21) | 3.91 |
| clústeres distintos | 8 y 20 | 4.33 | 2.48 | 1.15 (1.14–1.19) | 3.91 |
| clústeres distintos | 9 y 13 | 4.43 | 2.71 | 1.17 (1.05–1.24) | 3.82 |
| P ↔ P | 2 y 3 | 4.15 | 2.93 | 1.00 (0.98–1.02) | 4.01 |
| P ↔ P | 4 y 5 | 4.30 | 3.18 | 1.06 (1.00–1.18) | 4.04 |
| P ↔ E | 2 y 12 | 4.34 | 2.70 | 1.09 (1.05–1.24) | 4.00 |

## Efecto de separar los hilos entre «sockets» (nativo, bench.cpp del equipo)

Cambio de rendimiento de `E-compacto` a `E-disperso`, en %. «≈» = los rangos de las rondas se solapan: no se distingue del ruido.


**2 hilos**

| Panel | Autores | Libstdc++ | Williams | Mutex |
|---|---|---|---|---|
| n = 1, u = 1 % | -46 % | -38 % | -32 % | +50 % |
| n = 1, u = 10 % | -50 % | -37 % | -40 % | +23 % |
| n = 1, u = 50 % | -50 % | -45 % | -44 % | ≈+3 % |
| n = 10, u = 1 % | -36 % | -38 % | -32 % | -40 % |
| n = 10, u = 10 % | -43 % | -35 % | -28 % | -39 % |
| n = 10⁶, u = 50 % | +1 % | -5 % | ≈+0 % | ≈-0 % |

**4 hilos**

| Panel | Autores | Libstdc++ | Williams | Mutex |
|---|---|---|---|---|
| n = 1, u = 1 % | -35 % | -19 % | ≈-1 % | -21 % |
| n = 1, u = 10 % | -47 % | -20 % | -43 % | -26 % |
| n = 1, u = 50 % | -48 % | -16 % | -53 % | -26 % |
| n = 10, u = 1 % | -51 % | -52 % | -42 % | -45 % |
| n = 10, u = 10 % | -50 % | -48 % | -40 % | -44 % |
| n = 10⁶, u = 50 % | +2 % | ≈+1 % | +1 % | +2 % |

**8 hilos**

| Panel | Autores | Libstdc++ | Williams | Mutex |
|---|---|---|---|---|
| n = 1, u = 1 % | +19 % | ≈-3 % | ≈+78 % | -6 % |
| n = 1, u = 10 % | ≈-4 % | ≈-3 % | ≈-20 % | -11 % |
| n = 1, u = 50 % | -10 % | ≈+16 % | ≈-26 % | ≈-5 % |
| n = 10, u = 1 % | ≈+0 % | -9 % | ≈+9 % | -13 % |
| n = 10, u = 10 % | ≈-5 % | ≈-5 % | +3 % | -12 % |
| n = 10⁶, u = 50 % | +2 % | ≈-0 % | +0 % | +1 % |

**12 hilos**

| Panel | Autores | Libstdc++ | Williams | Mutex |
|---|---|---|---|---|
| n = 1, u = 1 % | ≈+3 % | ≈+20 % | +43 % | -3 % |
| n = 1, u = 10 % | ≈-3 % | ≈+3 % | ≈-4 % | ≈-2 % |
| n = 1, u = 50 % | ≈-2 % | ≈-2 % | ≈+33 % | -5 % |
| n = 10, u = 1 % | ≈+8 % | ≈+5 % | ≈-3 % | -4 % |
| n = 10, u = 10 % | ≈-17 % | ≈-0 % | ≈+3 % | -3 % |
| n = 10⁶, u = 50 % | +0 % | ≈-1 % | +0 % | +1 % |

## Alineación de las pilas en bench.cpp

Mop/s, mediana de 3 rondas.

| Impl | n | u | Hilos | sin alinear | alineado | alineado / sin alinear |
|---|---|---|---|---|---|---|
| Autores | 1 | 1 | 6 | 20.88 | 20.39 | 0.98× |
| Autores | 1 | 1 | 12 | 22.04 | 22.30 | 1.01× |
| Autores | 1 | 1 | 24 | 20.81 | 21.16 | 1.02× |
| Autores | 10 | 1 | 6 | 32.21 | 68.93 | 2.14× |
| Autores | 10 | 1 | 12 | 30.76 | 88.38 | 2.87× |
| Autores | 10 | 1 | 24 | 32.48 | 118.56 | 3.65× |
| Autores | 10 | 10 | 6 | 22.71 | 42.19 | 1.86× |
| Autores | 10 | 10 | 12 | 21.60 | 50.61 | 2.34× |
| Autores | 10 | 10 | 24 | 17.88 | 56.21 | 3.14× |
| Libstdc++ | 1 | 1 | 6 | 2.71 | 2.76 | 1.02× |
| Libstdc++ | 1 | 1 | 12 | 1.52 | 1.60 | 1.05× |
| Libstdc++ | 1 | 1 | 24 | 0.93 | 0.94 | 1.02× |
| Libstdc++ | 10 | 1 | 6 | 13.62 | 27.76 | 2.04× |
| Libstdc++ | 10 | 1 | 12 | 11.00 | 19.70 | 1.79× |
| Libstdc++ | 10 | 1 | 24 | 6.75 | 8.18 | 1.21× |
| Libstdc++ | 10 | 10 | 6 | 10.76 | 21.18 | 1.97× |
| Libstdc++ | 10 | 10 | 12 | 7.69 | 9.16 | 1.19× |
| Libstdc++ | 10 | 10 | 24 | 3.44 | 3.66 | 1.06× |
| Mutex | 1 | 1 | 6 | 7.41 | 7.24 | 0.98× |
| Mutex | 1 | 1 | 12 | 7.44 | 7.39 | 0.99× |
| Mutex | 1 | 1 | 24 | 7.06 | 7.26 | 1.03× |
| Mutex | 10 | 1 | 6 | 19.73 | 23.67 | 1.20× |
| Mutex | 10 | 1 | 12 | 18.86 | 24.66 | 1.31× |
| Mutex | 10 | 1 | 24 | 17.73 | 23.34 | 1.32× |
| Mutex | 10 | 10 | 6 | 16.09 | 20.12 | 1.25× |
| Mutex | 10 | 10 | 12 | 14.77 | 20.45 | 1.39× |
| Mutex | 10 | 10 | 24 | 12.96 | 20.29 | 1.57× |
| Williams | 1 | 1 | 6 | 5.43 | 5.38 | 0.99× |
| Williams | 1 | 1 | 12 | 4.95 | 4.72 | 0.95× |
| Williams | 1 | 1 | 24 | 3.66 | 3.73 | 1.02× |
| Williams | 10 | 1 | 6 | 13.66 | 24.61 | 1.80× |
| Williams | 10 | 1 | 12 | 13.66 | 28.76 | 2.11× |
| Williams | 10 | 1 | 24 | 10.60 | 26.20 | 2.47× |
| Williams | 10 | 10 | 6 | 9.42 | 16.05 | 1.70× |
| Williams | 10 | 10 | 12 | 7.71 | 12.89 | 1.67× |
| Williams | 10 | 10 | 24 | 5.64 | 8.48 | 1.50× |

Todos los cocientes: mediana 1.31×, mínimo 0.95×, máximo 3.65×.

## Mismo código dentro y fuera de Docker

Mop/s, mediana de 3 rondas.

| Impl | n | u | Hilos | fuera | dentro | dentro / fuera |
|---|---|---|---|---|---|---|
| Autores | 1 | 1 | 1 | 57.09 | 64.23 | 1.12× |
| Autores | 1 | 1 | 24 | 20.67 | 20.68 | 1.00× |
| Autores | 1 | 10 | 1 | 40.82 | 45.21 | 1.11× |
| Autores | 1 | 10 | 24 | 4.94 | 5.02 | 1.02× |
| Autores | 1 | 50 | 1 | 18.02 | 19.48 | 1.08× |
| Autores | 1 | 50 | 24 | 1.16 | 1.15 | 0.98× |
| Autores | 10 | 1 | 1 | 63.59 | 63.40 | 1.00× |
| Autores | 10 | 1 | 24 | 34.55 | 31.41 | 0.91× |
| Autores | 10 | 10 | 1 | 44.97 | 44.41 | 0.99× |
| Autores | 10 | 10 | 24 | 18.44 | 18.39 | 1.00× |
| Autores | 1000000 | 50 | 1 | 5.96 | 5.83 | 0.98× |
| Autores | 1000000 | 50 | 24 | 78.83 | 79.04 | 1.00× |
| Libstdc++ | 1 | 1 | 1 | 41.41 | 42.98 | 1.04× |
| Libstdc++ | 1 | 1 | 24 | 0.80 | 0.84 | 1.05× |
| Libstdc++ | 1 | 10 | 1 | 30.29 | 31.38 | 1.04× |
| Libstdc++ | 1 | 10 | 24 | 0.38 | 0.40 | 1.05× |
| Libstdc++ | 1 | 50 | 1 | 13.84 | 14.26 | 1.03× |
| Libstdc++ | 1 | 50 | 24 | 0.10 | 0.10 | 1.01× |
| Libstdc++ | 10 | 1 | 1 | 42.51 | 42.89 | 1.01× |
| Libstdc++ | 10 | 1 | 24 | 6.44 | 6.67 | 1.04× |
| Libstdc++ | 10 | 10 | 1 | 31.17 | 31.16 | 1.00× |
| Libstdc++ | 10 | 10 | 24 | 3.07 | 3.44 | 1.12× |
| Libstdc++ | 1000000 | 50 | 1 | 5.94 | 5.71 | 0.96× |
| Libstdc++ | 1000000 | 50 | 24 | 85.95 | 84.65 | 0.98× |
| Mutex | 1 | 1 | 1 | 33.97 | 38.41 | 1.13× |
| Mutex | 1 | 1 | 24 | 7.50 | 7.55 | 1.01× |
| Mutex | 1 | 10 | 1 | 27.86 | 31.04 | 1.11× |
| Mutex | 1 | 10 | 24 | 4.84 | 5.00 | 1.03× |
| Mutex | 1 | 50 | 1 | 15.24 | 16.82 | 1.10× |
| Mutex | 1 | 50 | 24 | 2.00 | 2.09 | 1.05× |
| Mutex | 10 | 1 | 1 | 37.81 | 38.49 | 1.02× |
| Mutex | 10 | 1 | 24 | 19.01 | 18.67 | 0.98× |
| Mutex | 10 | 10 | 1 | 30.72 | 31.08 | 1.01× |
| Mutex | 10 | 10 | 24 | 13.95 | 13.95 | 1.00× |
| Mutex | 1000000 | 50 | 1 | 5.01 | 4.84 | 0.97× |
| Mutex | 1000000 | 50 | 24 | 66.85 | 64.36 | 0.96× |
| Williams | 1 | 1 | 1 | 28.18 | 29.46 | 1.05× |
| Williams | 1 | 1 | 24 | 4.07 | 4.24 | 1.04× |
| Williams | 1 | 10 | 1 | 16.68 | 17.58 | 1.05× |
| Williams | 1 | 10 | 24 | 0.89 | 0.88 | 0.98× |
| Williams | 1 | 50 | 1 | 5.91 | 6.27 | 1.06× |
| Williams | 1 | 50 | 24 | 0.18 | 0.19 | 1.03× |
| Williams | 10 | 1 | 1 | 29.00 | 29.42 | 1.01× |
| Williams | 10 | 1 | 24 | 11.26 | 12.52 | 1.11× |
| Williams | 10 | 10 | 1 | 17.25 | 17.40 | 1.01× |
| Williams | 10 | 10 | 24 | 5.61 | 5.98 | 1.07× |
| Williams | 1000000 | 50 | 1 | 2.87 | 2.86 | 0.99× |
| Williams | 1000000 | 50 | 24 | 52.33 | 52.40 | 1.00× |

Todos los cocientes: mediana 1.02×, mínimo 0.91×, máximo 1.13×.

## Williams en bench.cpp, dentro de Docker: std::atomic frente a boost::atomic

Mop/s, mediana de 3 rondas, n = 1.

| Núcleos | u | Hilos | std::atomic | boost::atomic |
|---|---|---|---|---|
| 16,20 | 50 | 2 | 1.19 | 1.22 |
| 8,12 | 50 | 2 | 1.12 | 1.15 |
| 8,9 | 50 | 2 | 2.03 | 2.20 |
| 9,13 | 50 | 2 | 1.16 | 1.28 |
| sistema | 1 | 24 | 4.29 | 5.36 |
| sistema | 10 | 24 | 0.88 | 1.07 |
| sistema | 50 | 24 | 0.20 | 0.23 |

## Latencia de Williams: con candado oculto (sin -mcx16) y sin él

Medianas en ns.

| Operación | p | Con candado | Sin candado |
|---|---|---|---|
| load | 1 | 52 | 34 |
| load | 24 | 40680 | 3104 |
| load | 56 | 36820 | 23738 |
| store | 1 | 39 | 32 |
| store | 24 | 4420 | 1021 |
| store | 56 | 5410 | 4105 |
| store & reclaim | 1 | 51 | 44 |
| store & reclaim | 24 | 4594 | 821 |
| store & reclaim | 56 | 5352 | 4439 |
