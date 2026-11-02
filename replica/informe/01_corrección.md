# Corrección (ASan, ThreadSanitizer, variantes rotas, intentos de CAS, simulador)

Responsable: David. Ver ../../RESPONSABILIDADES.md.

## Método

Afirmaciones del paper que se comprueban: el algoritmo elimina la carrera read-reclaim (§2.2) y `increase_weight` termina en como mucho P intentos de CAS (§4.1–4.2, con R-1: C ≥ 2P, y R-2).

Equipo: Intel i5-10400 (6 núcleos/12 hilos, 1 socket), WSL2 Ubuntu 24.04, g++ 13.3.0.

1. **Estrés con sanitizadores** (`codigo/estres_asan.cpp`): varios hilos leen y escriben la misma celda (10 % store, 90 % load). Se compara una versión ingenua (leer el puntero y después incrementar el contador) con el código original de los autores (submódulo `codigo/autores`, commit 8540c58). Se compila con AddressSanitizer y con ThreadSanitizer (`codigo/correr_tsan.sh`), con 8, 12 y 16 hilos.
2. **Boceto didáctico** (`codigo/boceto/`, P = 8, C = 16) con dos variantes rotas: sin salida temprana (`-DSIN_SALIDA`) y C = P (`-DVAR_C=8`, incumple R-1). Mide el máximo de intentos de CAS por recarga.
3. **Simulador** (`simulador/buscar.js`): explora entrelazados aleatorios (20 000 ejecuciones por variante) y comprueba invariantes tras cada paso atómico.

## Resultados

| Prueba | Versión | Resultado | Fuente |
|---|---|---|---|
| ASan, 8 hilos | ingenua | *heap-use-after-free* en `Ingenua::load()` | `capturas/02_asan_ingenua.txt` |
| ASan, 8/12/16 hilos (1,6/2,4/3,2 M op.) | autores | 0 errores (53/68/92 ms) | `resultados/david-i5-10400/03_asan_autores_hilos.txt` |
| TSan, 8 y 16 hilos | autores | 0 avisos | `resultados/david-i5-10400/09_tsan.txt` |
| TSan, 8 hilos | ingenua | *data race* y *heap-use-after-free* | ídem |
| Boceto, 8 hilos, 4 M op. | paper | 0 errores; máx. 7 intentos de CAS (cota P = 8) | `capturas/04_boceto.txt` |
| Boceto | sin salida temprana | 0 errores de memoria, pero **38 intentos** > P | `capturas/05_boceto_variantes.txt` |
| Boceto | C = P | 50 060 errores, 3 055 objetos sin liberar | ídem |
| Simulador, P = 4, C = 8 | paper | 0 fallos en 1 909 884 pasos; intentos: 1 (80,1 %), 2 (18,2 %), 3 (1,7 %), 4 (0,06 %) | `capturas/08_simulador.txt` |
| Simulador | ingenua / umbral 0 / sin reserva / sin salida | 4 796 / 5 089 / 1 171 / 5 467 fallos | ídem |

Conclusión: se reproduce que la versión ingenua sufre la carrera y la del paper no, y que cada decisión de diseño (salida temprana, C ≥ 2P, reservar antes de recargar) es necesaria: quitarla rompe la corrección o la cota.

## Diferencias con el paper

- El paper **demuestra** la corrección y la cota (§4.1); aquí solo se **observa** que no se violan en las ejecuciones probadas.
- La cota de intentos se mide en el boceto y en el simulador, no en el código de los autores (no instrumentado; con P = 2¹⁶ y 16 hilos la recarga es rarísima).
- El paper no usa sanitizadores; son una herramienta propia de la réplica.
