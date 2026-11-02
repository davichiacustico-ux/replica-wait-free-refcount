# Rendimiento (figura 5)

Responsable: Jheremy. Ver ../../RESPONSABILIDADES.md.

> Borrador con la pasada de referencia de `capturas/` (i5-10400). Falta: 3 repeticiones por punto y un segundo equipo.

## Método

`codigo/bench.cpp rendimiento`: n pilas concurrentes con `atomic_shared_ptr`; una fracción u de las operaciones son actualizaciones (pop de una pila aleatoria y push en otra) y el resto lee la cima (§6.1). Seis paneles: (n, u) = (1, 1), (1, 10), (1, 50), (10, 1), (10, 10), (10⁵, 50). Hilos: 1, 2, 4, 8, 12 y 18 (18 = 1,5 × 12, como 216 = 1,5 × 144 en el paper). 1 s por punto.

## Resultados

Mop/s con 1 / 12 / 18 hilos. Fuente: `capturas/rendimiento.csv`, `graficos/local_rendimiento.pdf`.

| Panel | Autores | Libstdc++ | Mutex |
|---|---|---|---|
| n=1, u=1 | 44,7 / 34,3 / 38,3 | 34,1 / 2,2 / 1,5 | 25,3 / 6,4 / 6,0 |
| n=1, u=10 | 31,1 / 9,3 / 9,8 | 17,0 / 1,0 / 0,7 | 17,4 / 4,1 / 3,7 |
| n=1, u=50 | 12,4 / 2,2 / 2,2 | 9,6 / 0,3 / 0,2 | 10,2 / **2,9 / 2,5** |
| n=10, u=1 | 30,2 / 45,8 / 47,1 | 30,6 / 7,0 / 5,3 | 22,9 / 19,1 / 19,1 |
| n=10, u=10 | 30,5 / 28,9 / 28,9 | 21,7 / 7,9 / 4,4 | 18,4 / 14,4 / 13,9 |
| n=10⁵, u=50 | 4,3 / 29,8 / 29,0 | 4,0 / 19,6 / 18,4 | 3,5 / 17,4 / 18,9 |

Se reproduce la afirmación principal frente a la única implementación sin espacio extra disponible (Libstdc++): con varios hilos, Autores es mucho más rápido en todos los paneles. Con alta contención y muchas actualizaciones (n = 1, u = 50) gana el Mutex, coherente con lo que el paper admite: las actualizaciones usan CAS y se reintentan cuando coinciden con un `load`.

## Diferencias con el paper

- **Sin NUMA**: el paper usa 4 × Xeon E7-8867 (4 sockets, 144 hilos); aquí 1 socket, 12 hilos. Se comparan orden relativo y forma de las curvas, no valores absolutos (44,7 frente a ~9 Mop/s en n=1, u=1), y no se evalúa el coste de coherencia entre sockets.
- **Último panel**: n = 10⁵ en lugar de 10⁶.
- **Rivales**: faltan Williams y Folly (split reference count, lock-free) y Anderson; se añade Mutex, que el paper no usa. En el paper Anderson supera a «Our» con alta contención (n=1, u=1; n=1, u=10; n=10, u=10): esa parte no se contrasta.
- Una sola pasada (pendiente: 3 repeticiones con mediana y rango).
