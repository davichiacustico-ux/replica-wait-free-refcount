# Paper frente a réplica

Responsable: Diego (respaldo: David). Ver ../../RESPONSABILIDADES.md.

## Método

Cada afirmación del paper se contrasta con la prueba de la réplica que la cubre. Detalle en los apartados 01–04.

## Resultados

| Afirmación del paper | Prueba en la réplica | ¿Se reproduce? |
|---|---|---|
| Elimina la carrera read-reclaim (§2.2, §3) | ASan y TSan con 8–16 hilos; simulador | **Sí** (01) |
| `increase_weight` acaba en ≤ P intentos de CAS (§4) | Boceto (máx. 7 ≤ 8) y simulador (máx. 4 ≤ 4) | **Sí, empíricamente** (01) |
| Salida temprana y C ≥ 2P son necesarias | Variantes rotas: 38 intentos > P; 50 060 errores con C = P | **Sí** (01) |
| Más rendimiento que las implementaciones sin espacio extra (§6.1) | Frente a Libstdc++, 6 paneles | **Parcial**: solo frente a Libstdc++ (03) |
| Con alta contención y actualizaciones pierde ventaja (§6.1) | n=1, u=50: gana Mutex | **Sí** (03) |
| `load` con la menor latencia (§6.2) | Frente a Libstdc++ y Mutex | **Sí frente a Libstdc++; frente a Mutex solo en la cola p99** (02) |
| Sin sobrecoste de memoria | Pico de RSS | **Pendiente** (04) |

## Diferencias con el paper (limitaciones)

1. **Hardware.** La réplica se ejecutó en una sola CPU (Intel i5-10400, 6 núcleos/12 hilos, sin NUMA). El rendimiento del paper usó 4 sockets/144 hilos, así que comparamos orden relativo y forma de las curvas, no valores absolutos, y no evaluamos la coherencia entre sockets. La latencia del paper también usó un solo socket (Xeon w7-3465X): ahí la comparación es más directa salvo por el número de hilos (12 frente a 56).
2. **Rivales.** El paper compara con Libstdc++, Williams y Folly (split reference count, lock-free) y Anderson (hazard pointers). La réplica solo usa Libstdc++ y añade un Mutex. No se contrasta la afirmación frente al split reference count ni los casos en que Anderson supera al algoritmo. Queda como trabajo futuro (Williams es una sola cabecera: github.com/anthonywilliams/atomic_shared_ptr).
3. **Parámetros.** Último panel con n = 10⁵ en lugar de 10⁶; hilos hasta 18 (1,5 × 12) frente a 216 (1,5 × 144).
4. **Repeticiones.** Una pasada por punto.
5. **Cota de CAS.** Observada en el boceto y el simulador, no instrumentada en el código de los autores.
