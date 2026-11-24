# 01. Corrección y cota *wait-free*

Responsable original del apartado: David Tamo Turpo. Versión actualizada con la réplica completa en **Intel Core Ultra 9 285** (24 núcleos: 8 P + 16 E, Ubuntu 24.04 nativo y Docker Ubuntu 22.04) y **Intel Core i5-10400** (6 núcleos / 12 hilos SMT, WSL2 Ubuntu 24.04).

## 1. Afirmaciones del paper puestas a prueba

1. **Ausencia de carreras *read-reclaim* y de *use-after-free* (§2.2 y §3, afirmación 15):** ningún lector accede a un objeto ni a un bloque de control ya liberado mientras otros hilos ejecutan `store` concurrentemente.
2. **Cota *wait-free* en la recarga de peso (§4.1–4.2, afirmación 14):** bajo los requisitos $\text{R-1}$ ($C \ge 2P$) y $\text{R-2}$, `increase_weight` realiza como máximo $P$ intentos de `CAS` antes de terminar (ya sea porque su propio `CAS` prospera, porque cambió el puntero o por salida temprana al observar que el peso subió).

## 2. Método experimental

1. **Detectores dinámicos sobre el código de los autores (`codigo/estres_asan.cpp` y `codigo/correr_tsan.sh`):** carga con 10 % de `store` y 90 % de `load` sobre una misma celda compartida, comparando una versión ingenua de dos pasos frente al submódulo oficial `codigo/autores` (commit `8540c58`) compilado con AddressSanitizer (ASan, 8, 12 y 16 hilos) y ThreadSanitizer (TSan con `setarch -R`, 8 y 16 hilos) en ambas máquinas.
2. **Detectores sobre los rivales (`replica/completa/herramientas/sanitizers_rivales.sh`):** 26 ejecuciones combinando ASan + UBSan y TSan sobre `bench.cpp` (Autores, Libstdc++, Williams, Mutex) y ASan + UBSan sobre el binario oficial `bench_stack` en Docker (Autores, Libstdc++, Williams, Folly, Anderson).
3. **Boceto didáctico y variantes rotas (`codigo/boceto/`, $P = 8, C = 16$) y simulador de intercalados (`simulador/buscar.js`):** se desactivan selectivamente la salida temprana (`-DSIN_SALIDA`), el requisito $\text{R-1}$ (`-DVAR_C=8`, $C = P$), la reserva previa al `CAS` y el umbral $P$ para verificar que cada mecanismo del diseño es necesario.
4. **Instrumentación de la cota en el código real de los autores (`herramientas/cota_cas.cpp` y `correr_cota_cas.sh`):** sobre una copia al vuelo de la implementación real se cuentan las recargas efectivas y el máximo de intentos de `CAS` por llamada tanto con los parámetros de producción ($P = 65\,536, C = 524\,287$) como forzando recargas constantes ($P = 32, C = 64$, y el caso límite $P = 32, C = 32$).

## 3. Resultados verificados

### 3.1 Sanitizadores y variantes rotas en ambas máquinas

| Prueba | Versión / Algoritmo | Core i5-10400 (WSL2) | Core Ultra 9 285 (Linux nativo / Docker) | Fuente |
|---|---|---|---|---|
| ASan, 8 hilos | Ingenua (2 pasos) | *heap-use-after-free* inmediato | *double-free* / *heap-use-after-free* inmediato | `02_asan_ingenua.txt` |
| ASan, 8 / 12 / 16 hilos (1,6 / 2,4 / 3,2 M ops) | Autores (`codigo/autores`) | **0 errores** (53 / 68 / 92 ms) | **0 errores** (63 / 98 / 137 ms) | `03_asan_autores_hilos.txt` |
| TSan, 8 y 16 hilos | Autores (`codigo/autores`) | **0 avisos** | **0 avisos** | `09_tsan.txt` |
| Rivales: ASan+UBSan y TSan (26 ejecuciones) | Autores, Libstdc++, Williams, Folly, Anderson, Mutex | — | **0 avisos** en las 26 pruebas | `10_sanitizers_rivales.txt` |
| Boceto ($P=8, C=16$, 8 hilos, 4 M ops) | Paper completo | **0 errores**, 0 fugas, máx. **8** CAS ($\le P$) | **0 errores**, 0 fugas, máx. **6** CAS ($\le P$) | `04_boceto.txt` |
| Boceto sin salida temprana | `-DSIN_SALIDA` | 0 errores, pero **55** CAS ($> P$) | 0 errores, pero **45** CAS ($> P$) | `05_boceto_variantes.txt` |
| Boceto rompiendo $\text{R-1}$ ($C = P = 8$) | `-DVAR_C=8` | **63 101 errores**, 3 808 sin liberar | **42 348 errores**, 4 280 sin liberar | `05_boceto_variantes.txt` |
| Simulador ($P=3, C=6$, 20 000 ejec./variante) | Paper / Ingenua / Umbral 0 / Sin reserva / Sin salida | **0 fallos** en Paper (y 0 en 1 909 884 pasos con $P=4$); las 4 variantes rotas fallan 4 796 / 5 089 / 1 171 / 5 467 veces | — | `capturas/08_simulador.txt` |

### 3.2 Cota *wait-free* sobre el código real de los autores (`cota_cas.csv`, Ultra 9)

| Configuración | Hilos probados | Recargas observadas (máx. por punto) | Máx. intentos de `CAS` | Cota teórica $P$ | Errores |
|---|---|---|---|---|---|
| Parámetros reales ($P = 65\,536,\ C = 524\,287$) | 2 a 32 (0 %, 1 %, 10 % `store`) | hasta 1 158 (solo con 0 % `store`) | **4** | 65 536 | 0 |
| Recarga forzada ($P = 32,\ C = 64$, cumple $\text{R-1}$) | 2 a 32 (0 %, 1 %, 10 % `store`) | hasta 8 061 142 (64,4 M en total) | **8** | 32 | 0 |
| Caso límite ($P = 32,\ C = 32$, viola $\text{R-1}$) | 8 y 24 (1 % `store`) | hasta 15 600 117 | 10 | 32 | 0 *(con hilos $\le P$, pero recarga casi siempre y rinde ~la mitad)* |

En la configuración forzada ($P = 32, C = 64$) con 24 hilos y 1 % de `store` (7 023 182 recargas), el 84,0 % de las recargas se resolvió en **1 solo intento** (`5 902 378`), el 14,2 % en 2 intentos (`997 824`) y apenas 4 recargas llegaron a 8 intentos, muy por debajo de la cota $P = 32$.

## 4. Alcance y límites

- **Prueba empírica frente a demostración formal:** obtener 0 errores con ASan, UBSan y TSan confirma que los detectores estaban activos (la versión ingenua dispara el fallo de inmediato) y que la implementación publicada respeta sus invariantes en los escenarios probados, pero **no sustituye** la demostración formal de las secciones 4.1 y 4.2 del paper.
- **TSan en Folly y Anderson:** no se pasó TSan al binario oficial de Folly ni al de Anderson porque `libfolly.a` precompilada sin instrumentación produciría falsos positivos; sí pasaron ASan + UBSan sin avisos.
