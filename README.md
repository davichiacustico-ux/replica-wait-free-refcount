# Réplica: conteo de referencias wait-free (Euro-Par 2024)

Exposición y réplica del paper **«A Fast Wait-Free Solution to Read-Reclaim Races in Reference Counting»**
(I. G. de Wolff, D. Anderson, G. K. Keller y A. Seletskiy, Euro-Par 2024, LNCS 14803, págs. 103–118,
doi:10.1007/978-3-031-69583-4_8).

Equipo: David Tamo Turpo, Jheremy Manuel Alvarez Astete y Diego Quispe Candelario.
**Quién hace qué: [RESPONSABILIDADES.md](RESPONSABILIDADES.md).**

## Clonar

```bash
git clone --recursive <url-de-este-repositorio>
# si ya lo clonaste sin --recursive:
git submodule update --init
```

`codigo/autores` es un submódulo con el código original de los autores
([ivogabe/wait-free-atomic-shared-pointer](https://github.com/ivogabe/wait-free-atomic-shared-pointer), commit 8540c58).
No se copia aquí porque su repositorio no declara licencia.

## Estructura

| Carpeta / archivo | Qué hay |
|---|---|
| `exposicion_15min.pdf`, `exposicion_25min.pdf` | Diapositivas (fuente `.tex`) |
| `guion_15min.pdf`, `guion_25min.pdf`, `guion_expositores.pdf` | Qué decir en cada diapositiva |
| `codigo/` | Pruebas: `estres_asan.cpp` (la carrera con AddressSanitizer), `boceto/` (versión didáctica y variantes rotas), `bench.cpp` (rendimiento y latencia), `ejecutar_pruebas.sh` |
| `simulador/` | Simulador de intercalados que comprueba los invariantes del paper paso a paso (Node.js) |
| `capturas/` | Salidas de referencia (WSL Ubuntu 24.04, Intel i5-10400, 12 hilos) |
| `graficos/` | `hacer_graficos.py`: gráficas de rendimiento y latencia y capturas de terminal como PDF |
| `figuras/` | `extraer_figuras.py`: recorta del PDF del paper las figuras de las diapositivas (no se suben) |
| `practica_cpp/` | Lo mínimo de C/C++ para entender el paper, en 8 partes con ejercicios |
| `replica/informe/` | Un apartado por persona |
| `replica/resultados/` | Resultados de cada equipo: `<nombre>-<cpu>/` |

## Ejecutar las pruebas (Linux o WSL, g++ ≥ 12)

```bash
cd codigo
bash ejecutar_pruebas.sh                                   # salida en ../capturas
bash ejecutar_pruebas.sh ../replica/resultados/david-i5-10400   # salida en tu carpeta de resultados
```

El script compila y guarda: demo de los autores, AddressSanitizer (versión ingenua frente a la de los autores),
boceto y variantes, y `bench` en modo rendimiento y latencia (CSV). Tarda unos 3 minutos.

```bash
node simulador/buscar.js 20000 ingenua,sinReserva          # simulador de contraejemplos
python graficos/hacer_graficos.py                          # gráficas y capturas como PDF
python figuras/extraer_figuras.py "ruta/al/paper.pdf"      # figuras del paper para compilar las diapositivas
latexmk -pdf exposicion_25min.tex                          # diapositivas (MiKTeX o TeX Live)
```

## Resultados de referencia (i5-10400, 12 hilos)

- AddressSanitizer: la versión ingenua produce *heap-use-after-free*; el código de los autores, 1,6 M operaciones sin errores.
- Rendimiento con 1 pila, 1 % de actualizaciones y 12 hilos: autores 34,3 Mop/s, `std::atomic<std::shared_ptr>` de GCC 2,2, mutex 6,4.
- Latencia de `load` con 12 hilos (mediana / p99): autores 269 ns / 0,83 µs; GCC 3,06 µs / 16 µs; mutex 61 ns / 27,6 µs.
