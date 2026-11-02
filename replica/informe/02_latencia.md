# Latencia (figura 6)

Responsable: David. Ver ../../RESPONSABILIDADES.md.

## Método

`codigo/bench.cpp latencia` reproduce el banco del §6.2: p hilos; el primero se mide y los p − 1 restantes hacen `store` continuamente. Se miden `load`, `store` sin liberar y `store & reclaim`, con p = 1 (cada muestra es la media de 16 operaciones) y p = 12 (todos los hilos de la CPU). 100 000 muestras; se descuenta el coste del reloj (mediana 47,2 ns). Implementaciones: Autores, Libstdc++ (`std::atomic<std::shared_ptr>`, con candados) y Mutex (línea base propia).

## Resultados

Mediana (p99) en ns. Fuente: `capturas/07_latencia.txt`, `graficos/local_latencia.pdf`.

| Operación | p | Autores | Libstdc++ | Mutex | Paper: Ours / libstdc++ |
|---|---|---|---|---|---|
| load | 1 | 3,9 (32,2) | 11,6 (11,8) | 4,2 (4,3) | 23 / 29 |
| store | 1 | 11,9 (18,1) | 12,0 (12,1) | 4,3 (4,5) | 33 / 36 |
| store & reclaim | 1 | 30,3 (43,2) | 26,6 (39,7) | 19,8 (24,3) | 44 / 41 |
| load | 12 (paper 56) | 268,9 (825,8) | 3 060,3 (15 968,7) | 61,0 (27 625) | 1 140 / 86 700 |
| store | 12 | 209,0 (686,6) | 3 052,7 (15 767,2) | 152,2 (30 711,8) | 1 470 / 111 000 |
| store & reclaim | 12 | 222,1 (616,7) | 3 338,8 (20 426) | 187,0 (30 252,8) | 1 570 / 59 300 |

Se reproduce lo que el paper afirma frente a Libstdc++: `load` más rápido sin contención y más de un orden de magnitud más rápido con contención. En `store & reclaim` con p = 1, Libstdc++ es algo mejor, igual que en el paper.

## Diferencias con el paper

- **Mutex** (no está en el paper) tiene mejor **mediana** con contención, pero su cola p99 en `load` es ~33 veces peor que la de Autores. Sus percentiles p1 = p25 = 0,0 ns indican muestras por debajo de la resolución del reloj tras descontar su coste: sus medianas deben leerse con cautela. El paper afirma que su `load` es el más bajo «consistentemente»; frente a Mutex aquí solo se cumple en la cola.
- Contención p = 12 frente a p = 56. Ambos son una sola CPU (el w7-3465X del paper es de un socket): comparación directa salvo por el número de hilos.
- Faltan Folly y Anderson (en el paper Anderson gana en `store` con p = 1 y en `store & reclaim` con p = 56). Williams lo omite el propio paper en latencia.
- Valores absolutos no comparables entre máquinas; se comparan órdenes relativos. Una sola pasada.
