# 05. Comparación global: las 15 afirmaciones del paper frente a la réplica

Responsables originales del apartado: Diego Quispe Candelario y David Tamo Turpo. Versión integrada tras completar las **14 158 mediciones** sin fallos en el **Core Ultra 9 285** (con y sin Docker) y las pruebas locales en el **Core i5-10400**.

## 1. Veredicto afirmación por afirmación

**Marcador global:** **9 se reproducen · 2 en parte · 3 no se reproducen · 1 no se confirma.**

| # | Afirmación del paper | Evidencia medida en la réplica | Veredicto |
|---:|---|---|---|
| 1 | Mucho más rápido que Libstdc++ (§6.1) | Con contención, Libstdc++ cae a `0,06×–0,16×` en Docker (`0,05×–0,17×` sin Docker; `0,04×` en i5-10400); empatan en $n = 10^6$ | **Se reproduce** |
| 2 | Mucho más rápido que Williams (§6.1) | Autores gana en los 6 escenarios: Williams queda entre `0,24×` y `0,73×` en Docker (`0,25×–0,76×` sin Docker; `0,14×` en $n=1,u=1\%$ en i5) | **Se reproduce** |
| 3 | Mucho más rápido que Folly (§6.1) | En el Ultra 9 Autores gana 3 paneles, empata 2 y pierde 1 ($n=1, u=50\%$: `1,07` vs `1,47 Mop/s`). La brecha ya existe con **1 solo hilo** (`0,86×–1,29×` aquí frente a `0,18×–0,46×` en el paper) | **No se reproduce** |
| 4 | Con $u$ pequeño supera a Anderson (§6.1) | Con $u = 1\%$ gana `+6 %` en $n = 1$ (`+10 %` sin Docker) y `+65 %` en $n = 10$ (`121,9` vs `74,0 Mop/s`; `+75 %` sin Docker) | **Se reproduce** |
| 5 | Con $n$ grande ($10^6$) supera a Anderson (§6.1) | Pierde con pocos hilos y empata a cuatro bandas en `53–57 Mop/s` desde 24 hilos (la RAM de 1 canal llega al `89–93 %` de su techo aleatorio) | **No se reproduce** *(en este PC)* |
| 6 | Con mucha contención Anderson gana (§6.1) | Con $n = 1$ y $u \ge 10\%$, Anderson alcanza `1,87×–1,99×` de Autores (`1,90×–1,93×` sin Docker) | **Se reproduce** |
| 7 | `load` siempre de menor latencia (§6.2) | Menor mediana en $p = 1, 24, 56$ con y sin Docker (`14` y `414–437 ns`); con contención los rangos se solapan con Anderson y Folly | **Se reproduce** *(con poco margen)* |
| 8 | `store` segundo con $p = 1$, tras Anderson (§6.2) | Es cuarto de cinco (`22 ns` frente a Folly `17`, Anderson `18` y Libstdc++ `20 ns`), con y sin Docker | **No se reproduce** |
| 9 | `store` el mejor con contención (§6.2) | Empate técnico en cabeza con Anderson (`507` vs `483 ns` en Docker; `461` vs `470 ns` sin Docker) | **En parte** |
| 10 | `store & reclaim` más lento que `store` con $p = 1$ (§6.2) | Se cumple en los 5 algoritmos con y sin Docker (y también en `bench.cpp` en ambas máquinas) | **Se reproduce** |
| 11 | Con contención algunos mejoran en `store & reclaim` (§6.2) | Con 15 rondas sin Docker la diferencia queda entre `1 %` y `4 %` (`461` → `456 ns`), dentro del ruido entre rondas | **No se confirma** |
| 12 | Latencia de Williams casi siempre mayor (§6.2) | Con $p = 1$ es el más lento (`32–45 ns`), pero con contención y `-mcx16` supera a Libstdc++ (5–6 de 6 casos) y a veces a Folly (2–3 de 6) | **En parte** |
| 13 | Anderson tiene poco sobrecoste de memoria; Autores, ninguno (§6.1, §7) | Anderson retiene `19 960–20 256` bloques de control (`469–476 KB` con 24 hilos); Autores y los otros tres retienen `0` bloques diferidos (`2,3 KB` vivos) | **Se reproduce** |
| 14 | `increase_weight` reintenta como mucho $P$ veces (§4.2) | Código real instrumentado: máx. **8** intentos en 64,4 M recargas forzadas con $P = 32$ (y máx. **4** con $P = 65\,536$). Boceto sin salida temprana sube a 45–55 ($> P=8$) | **Se reproduce** |
| 15 | Sin *use-after-free* en carreras *read-reclaim* (§2.2, §4.1) | 0 errores con ASan, UBSan y TSan en ambas máquinas (mientras la versión ingenua y $C = P$ fallan masivamente) | **Se reproduce** *(en lo probado)* |

## 2. Aportaciones propias de la réplica

1. **Auditoría del candado oculto en Williams:** se detectó que el CMake oficial de latencia omitía `-mcx16`, degradando `boost::atomic` de 16 bytes a un candado (`40,7 µs` → `3,1 µs` al corregir).
2. **Cuantificación directa de memoria diferida y ancho de banda IMC:** se midió el pico exacto de bloques diferidos de Anderson (`~470 KB`) y su reflejo en el salto de cola `p99,95` (`~7,8 µs` cada 2 000 retiradas), así como el tráfico físico de RAM (`34 GB/s` sobre un techo aleatorio de `37 GB/s`).
3. **Aislamiento de factores de confusión:** se demostró experimentalmente que Docker no altera los binarios oficiales (259/270 puntos `< 10 %`), se cuantificó el impacto del recorte `2 s × 5` frente a `10 s × 7`, y se demostró el doble filo de `alignas(128)` en `bench.cpp` (`+3,40×` en $n = 10$ al eliminar compartición falsa, pero `-24 %` a `-27 %` en $n = 10^6$ al desbordar los 36 MB de caché L3).
4. **Sensibilidad a la topología de caché:** separar hilos fuera de una L2 compartida reduce el rendimiento de Autores a la mitad bajo contención, lo que también explica de forma coherente por qué un Core i5-10400 monolítico con SMT supera al Core Ultra 9 híbrido en $n = 1, u = 1\%$ a 12 hilos (`39,1` vs `21,8 Mop/s`).

## 3. Limitaciones declaradas y preguntas abiertas

- **Escala y hardware:** ninguna de las dos máquinas tiene 4 sockets ni memoria NUMA real (el castigo entre clústeres E es de `1,35×` frente a `~3×` entre sockets Xeon), y el Ultra 9 operó con un solo canal de RAM y sin aislamiento de núcleos (`isolcpus`) para latencia bajo contención.
- **Preguntas abiertas:** (a) por qué Folly rinde cinco veces menos a 1 hilo en la máquina del paper que aquí; (b) por qué Williams en `bench_stack` sufre una caída severa entre clústeres E (`-97 %` en Docker, `-76 %` sin Docker); (c) por qué Autores rinde menos que Anderson y Libstdc++ con pocos hilos en $n = 10^6$ pese a mover menos bytes por operación; y (d) confirmar con un segundo módulo DDR5 en el Ultra 9 cuánto sube el techo de $n = 10^6$.
