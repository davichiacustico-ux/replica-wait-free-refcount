# Traspaso: cómo se cerró la réplica

Para quien retome este trabajo, sea persona o agente. Todo lo necesario está en este repositorio. Si solo vas a leer un archivo además de este, que sea [EXPLICACION.md](EXPLICACION.md), sobre todo sus Partes 0, 7 y 8.

## Situación: cerrada el 7 de noviembre de 2026

**No queda nada obligatorio por medir.** Lo que sigue abierto son preguntas, al final de este archivo.

Así se cerró:

1. **23:07 del día 6 a 03:51 del día 7:** `bash replica/completa/pendientes.sh` corrió de principio a fin, en 4 h 44 min y sin un solo fallo. Hizo, en orden:
   - `preparar.sh`: compiló los binarios oficiales fuera de Docker.
   - `correr_nativo.sh oficial`: midió los 5 algoritmos sin Docker.
   - `correr_nativo.sh extra`: subió la latencia a 15 rondas y los pares de núcleos a 10.
   - `correr_nativo.sh alineado`: repitió `bench.cpp` con las pilas alineadas.
   - `analizar.py`: regeneró `tablas.md` y las figuras.
2. **4:30:** `revisar_al_amanecer.sh` comprobó los datos con `herramientas/comprobar_final.py` (todo completo), hizo copia en `bin/respaldo/` y lanzó un agente (`claude -p`). Ese agente respondió las 7 preguntas de abajo y actualizó EXPLICACION.md, README.md y el LEEME de resultados. No tocó datos ni código, y la huella SHA-256 lo confirma. Su resumen está en `bin/resumen_agente.md` y el registro en `bin/revision.log`.
3. **Por la mañana**, ya con alguien delante:
   - Se comprobaron las cifras principales del agente contra los CSV, y cuadran.
   - Se corrigió la errata que él señaló: el título de la tabla de pares decía «3 rondas» cuando eran 10. Ahora `analizar.py` cuenta las rondas, y al regenerar solo cambió esa línea, con las figuras idénticas byte a byte.
   - Se midió por qué alinear las pilas frena con n = 10⁶, con `herramientas/alinear_memoria.py`: las pilas alineadas dejan de caber en la caché L3. Está en la Parte 6.6 de EXPLICACION.md.

## Las 7 preguntas, ya respondidas

| # | Pregunta | Respuesta corta | Dónde está |
|---|---|---|---|
| 1 | ¿Williams se hunde entre clústeres sin Docker? | Sí, pero menos: −76 % (de 2,93 a 0,70 Mop/s) frente a −97 % con Docker | Partes 0 y 6.4 |
| 2 | ¿Sigue la diferencia de Folly con el paper? | Sí, con y sin Docker, y también con la receta exacta | Partes 3.4, 6.1 y 7 |
| 3 | ¿Docker cambia los binarios oficiales? | No: 259 de 270 puntos cambian menos de un 10 % | Partes 6.6 y 8 |
| 4 | ¿Se sostienen las filas 7, 9, 11 y 12 con 15 rondas? | La 7 y la 9, por poco; la 12, en parte; la 11 pasa a «No se confirma» | Partes 6.2 y 7 |
| 5 | ¿Cuánto se desvían los recortes? | De −10 % a +19 %, con un sesgo: dan más con contención y menos con n = 10⁶ | Partes 6.7 y 8 |
| 6 | ¿Cuánto mejoran los paneles n = 10 al alinear? | Autores, 3,40× y 3,28×. Con n = 10⁶, alinear frena un 24-27 %, y ya se sabe por qué | Parte 6.6 |
| 7 | ¿La memoria retenida es igual sin Docker? | Sí: Anderson llega a 20 256 bloques; los demás, bajo el límite | Parte 6.3 |

**Una decisión que es del equipo:** la fila 11 de la Parte 7 pasó de «Se reproduce» a «No se confirma». Con 15 rondas, la mejora de Autores se queda en un 1-4 %, dentro del ruido. Si preferís «Dudoso», es cambiar una palabra.

## Reglas, por si hay que volver a medir o a editar

- **Una medición a la vez, con la máquina libre.** Compilar o medir a la vez estropea los datos. Tampoco edites un `.sh` mientras corre.
- **No toques** `codigo/bench.cpp` ni `replica/informe/`. Son del equipo. Si ves un error, repórtalo.
- **No hagas commit ni push** sin que el usuario lo pida.
- **No escribas contraseñas** en ningún archivo.
- **Cada cifra que escribas debe salir de `tablas.md` o de un CSV.** No la copies de memoria ni la redondees a tu favor.
- **Si un dato contradice el documento, cambia el documento.** Los resultados malos se cuentan igual que los buenos.
- **Distingue lo medido de lo supuesto.** Una causa que no se midió es una hipótesis, y debe decirse.
- **Estilo:** español, para principiantes, con frases cortas y ejemplos cotidianos. Repetir una idea de varias formas es bienvenido. Mira cómo está escrito EXPLICACION.md.

## Cómo comprobar que una tanda nueva salió bien

```bash
cd replica/completa
tail -3 bin/pendientes.log                    # la última línea debe decir PENDIENTES TERMINADOS
python3 herramientas/comprobar_final.py       # sin IA: filas, códigos de salida, temperaturas y figuras
```

Si la cola se paró, relanza `bash replica/completa/pendientes.sh`: salta lo ya medido. Solo `basura` y `paridad` empiezan de cero, unos 40 minutos.

Filas esperadas, sin contar la cabecera:

| Archivo, dentro de `resultados/david-ultra9-285/` | Filas |
|---|---|
| `nativo_oficial/rendimiento.csv` | 3 400 (`fig5` 1 350, `afinidad` 1 350, `pares` 700) |
| `nativo_oficial/latencia.csv` | 1 260 (`fig6` 675, `latafin` 585) |
| `nativo_oficial/basura.csv` | 75 |
| `nativo_oficial/paridad.csv` | 30 |
| `nativo_alineado/rendimiento.csv` | 2 160 (`fig5` 1 080, `afinidad` 1 080) |
| `nativo_alineado/latencia.csv` | 0: a propósito, el modo `alineado` solo mide rendimiento |
| `nativo_alineado/memoria_alineado.csv` | 48 |

Los CSV terminan en CRLF, así que usa el módulo `csv` de Python y no `awk`: `awk` da falsos fallos. `pandas` no está instalado.

## Trampas que ya se pisaron

- **Docker y sin Docker no son solo eso.** Cambian más cosas, así que una diferencia no significa por sí sola que «Docker influye». Comprueba si otro factor puede explicarla:

  | | Con Docker | Sin Docker |
  |---|---|---|
  | g++-12 | 12.3.0 | 12.4.0 |
  | glibc | 2.35 | 2.39 |
  | Boost | 1.74 | 1.83 |
  | jemalloc | 5.2.1 | 5.3.0 |
  | Compilación de Folly | `getdeps.py` | CMake directo |

- **Williams tiene dos binarios de latencia.** `build/` lleva un candado escondido y `build_cx16/` no, y se mide con este último. Comprobado en el binario sin Docker: 8 `cmpxchg16b` y 0 `lock_pool`. Los otros cuatro algoritmos usan `build/`.
- **`latencia_williams_con_candado.csv` son datos descartados.** No los uses.
- **Los paneles n = 10 de `nativo/` están frenados por compartición falsa.** Compara con `nativo_alineado/`. Pero con n = 10⁶ es al revés: alinear frena, porque las pilas dejan de caber en la L3.
- **`bench.cpp` y los binarios oficiales son programas distintos.** No restes uno del otro sin leer la Parte 6.6.
- **Una medición puede engañar aunque salga sin errores.** Dos veces pasó aquí: Williams se midió con un candado, y la RAM se midió mientras el programa liberaba memoria. Si un número no encaja con lo que debería pasar, investígalo antes de creerlo.

## Lo que sigue abierto

Son preguntas, no tareas pendientes. Si alguien las ataca, que no invente una causa sin medirla:

- **Folly** va muy distinto del paper, con y sin Docker y con la receta exacta.
- **Autores con n = 10⁶ y pocos hilos** va por detrás. Ya se descartó que sea por mover más bytes.
- **Algunos algoritmos ganan al separar sus hilos.**
- **Williams entre clústeres** se hunde más con Docker (−97 %) que sin él (−76 %). Para saber si es Boost (1.74 frente a 1.83), habría que compilar fuera con 1.74, o dentro con 1.83.
- **Folly con 8 hilos compactos** se hunde en algunas rondas: de 0,35 a 1,33 Mop/s sin Docker.
- **El sesgo del recorte** (2 s × 5 frente a 10 s × 7) no tiene explicación medida.
- **Con `bench.cpp` y n = 10⁶, 10 s rinde menos que 2 s:** 65,1 frente a 78,8 Mop/s en Autores sin alinear. Salió de pasada en `alinear_memoria.py` y no se investigó.
- **La RAM como techo con n = 10⁶:** cada vez hay más indicios, pero para demostrarlo haría falta un segundo módulo de RAM.
- **4 sockets reales y NUMA:** este PC no los tiene.

## Git

Rama `replica-completa`. `origin` es `dtamotu/replica-wait-free-refcount` y `upstream` es `davichiacustico-ux/replica-wait-free-refcount`. El último commit es `e208e76`. Los cambios de esta fase siguen sin commitear, y `upstream` todavía no tiene la réplica. Espera a que el usuario pida el commit.
