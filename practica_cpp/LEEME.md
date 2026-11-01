# C y C++ mínimo para explicar el paper

Lo justo de C/C++ para entender y exponer *A Fast Wait-Free Solution to Read-Reclaim Races in
Reference Counting* (de Wolff, Anderson, Keller y Seletskiy, Euro-Par 2024), en ocho partes
cortas. Cada parte tiene tres archivos:

- `1_ejemplo.cpp`: el concepto, comentado línea a línea y conectado con el paper. Léelo y ejecútalo.
- `2_ejercicios.cpp`: ejercicios mínimos con `// TODO`. Al ejecutarlo, cada comprobación dice
  `[OK]` o `[FALTA]`. Al principio todas dicen `[FALTA]`.
- `3_solucion.cpp`: la solución. Mírala solo después de intentarlo.

## Cómo compilar y ejecutar

En VS Code, con el archivo abierto: **Ctrl+Shift+B** («Compilar y ejecutar», definido en
`.vscode/tasks.json`). Desde una terminal:

```bash
g++ -std=c++20 -O2 -Wall -pthread 1_ejemplo.cpp -o ejemplo
./ejemplo
```

## Las partes

| Parte | Qué aprendes | Dónde aparece en el paper |
|---|---|---|
| `01_punteros` | Puntero = dirección; `new`/`delete`; puntero colgante y use-after-free; un contador a mano | Sección 2.2: la carrera termina en un use-after-free |
| `02_bits` | `uint64_t`, hexadecimal, `<<`, `>>`, máscaras, `pack`/`unpack`, aritmética que da la vuelta | Sección 3.2 y figura 3: el peso en los 16 bits altos |
| `03_shared_ptr` | `std::shared_ptr`, `use_count`, destructor, `weak_ptr`, constructor de *aliasing* | Secciones 2.1 y 5.2: shared pointer, bloque de control, aliasing |
| `04_hilos` | `std::thread`, `join`, lambdas, carrera de datos con `x++`, `std::mutex` | Secciones 1 y 2: hilos, carreras, candados (Libstdc++) |
| `05_atomicos` | `std::atomic`: `fetch_add`, `exchange` (swap), `compare_exchange_strong` (CAS), bucle CAS | Algoritmos 1 y 2: las tres instrucciones del paper |
| `06_atomic_shared_ptr` | `std::atomic<std::shared_ptr<T>>` de C++20: `load`, `store`, `exchange`, CAS | Lo que el paper implementa; en GCC usa candados |
| `07_carrera_ingenua` | La carrera read-reclaim de verdad: leer el puntero y después sumar 1 | Sección 1 y 2.2: el problema |
| `08_peso_y_recarga` | El algoritmo del paper en ~40 líneas: `store`, `load`, `increase_weight` y una prueba con 8 hilos | Secciones 3 y 4: algoritmos 1 y 2, R-1, R-2 |

Orden recomendado: 01 → 08. Si ya sabes punteros y bits, empieza en la 03.

## Lo mínimo que debes poder decir en la exposición

1. Un `shared_ptr` cuenta referencias; el objeto se libera cuando el contador llega a 0 (parte 03).
2. Leer una variable `shared_ptr` compartida son dos pasos: leer el puntero y sumar 1 al contador,
   que está en otro sitio de la memoria. Entre los dos pasos otro hilo puede liberar el objeto (parte 07).
3. Las instrucciones atómicas (`fetch_add`, `exchange`, CAS) trabajan sobre **una** palabra (parte 05).
4. El truco: guardar el peso en los 16 bits altos del puntero (parte 02), para que leer sea un solo
   `fetch_sub` que resta un ticket y devuelve el puntero a la vez (parte 08).
5. La recarga es un bucle CAS que sale en cuanto ve que otro hilo ya recargó: como mucho P intentos (parte 08).

Los programas de las partes 04, 07 y 08 usan varios hilos: sus números cambian en cada ejecución.
Los de la 07 (versión ingenua) muestran errores a propósito: es la carrera que el paper resuelve.
