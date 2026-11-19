# Réplica completa: mapa de archivos

Esta carpeta contiene todo lo necesario para repetir los experimentos del paper **«A Fast Wait-Free Solution to Read-Reclaim Races in Reference Counting»** (Euro-Par 2024) frente a sus cuatro rivales, con y sin Docker, más una batería de experimentos con afinidad de núcleo para simular los 4 sockets de los Xeon con una sola CPU.

- **Para entender qué se hizo y qué salió**, lee [EXPLICACION.md](EXPLICACION.md). Explica desde cero el problema, cada algoritmo, el método y los resultados.
- **Para saber qué hace cada archivo y en qué orden usarlo**, sigue leyendo este README.
- **Los resultados** están en [../resultados/david-ultra9-285/](../resultados/david-ultra9-285/LEEME.md).
- **Para preparar la exposición y las preguntas del jurado**, mira [DEFENSA.md](DEFENSA.md): un tribunal simulado con 14 críticas, qué responder a cada una y qué mejorar.
- **Para revisar cómo se cerró la última fase** (comprobaciones y trampas conocidas), mira [TRASPASO.md](TRASPASO.md). Ya no queda nada por medir.

## Qué ejecutar según lo que quieras

| Quiero… | Ejecuto | Tarda | Estado |
|---|---|---|---|
| Los 5 algoritmos con los programas oficiales, dentro de Docker | `bash correr_docker.sh` | unas 3 h | Probado, con la imagen y el artefacto ya presentes. No se probó desde cero |
| Solo una parte de lo anterior | `bash correr_docker.sh pares` (o `fig6,latafin`, etc.) | varía | Probado |
| Autores, Libstdc++, Williams y Mutex sin Docker, con `bench.cpp` del equipo | `SOLO_DESCARGA=1 bash preparar.sh` y luego `bash correr_nativo.sh equipo` | unas 1,5 h | Probado |
| Los programas oficiales y la versión alineada sin Docker, de una vez: compila los binarios oficiales, los mide, repite lo inestable y regenera las tablas | `bash pendientes.sh` y después `python3 herramientas/comprobar_final.py` | unas 5 h | Probado de principio a fin la noche del 6 al 7 de noviembre de 2026: 4 h 44 min, sin fallos, con Folly y ParlayLib ya compilados de antes. Deja el registro en `bin/pendientes.log` |
| Solo los 5 algoritmos sin Docker | `bash preparar.sh 12` y luego `bash correr_nativo.sh oficial` | unos 10 min de compilación y 2 h 50 min de medición | Probado, dentro de `pendientes.sh` |
| Más rondas de latencia y de pares sobre lo anterior | `bash correr_nativo.sh extra` | unos 30 min | Probado, dentro de `pendientes.sh` |
| Quitar la compartición falsa de los paneles n = 10 sin Docker | `bash correr_nativo.sh alineado` | 1 h 20 min | Probado, dentro de `pendientes.sh` |
| Saber por qué difieren las dos versiones | `bash controles.sh` y luego `bash control_williams.sh` | unos 20 min y 2 min | Probado |
| Medir los bytes reales que entran y salen de la RAM | `python3 herramientas/medir_memoria.py` | unos 20 min | Probado |
| Saber por qué alinear las pilas frena con n = 10⁶ | `python3 herramientas/alinear_memoria.py` | unos 10 min | Probado |
| Pasar ASan, UBSan y TSan a los rivales | `bash herramientas/sanitizers_rivales.sh archivo_de_salida` | unos 10 min | Probado |
| Regenerar tablas y gráficas | `python3 analizar.py` | unos 4 s | Probado |

Todas las mediciones se **reanudan solas**: si algo se corta, vuelve a lanzar la misma orden y salta lo ya medido.

## Archivos

### Orquestación

| Archivo | Para qué sirve |
|---|---|
| [Dockerfile](Dockerfile) | La imagen `replica-wait-free`: Ubuntu 22.04 y g++-12, igual que el artefacto oficial |
| [correr_docker.sh](correr_docker.sh) | Un solo comando: construye la imagen, baja el artefacto de Zenodo, comprueba su MD5, compila y mide |
| [construir_en_docker.sh](construir_en_docker.sh) | Se ejecuta **dentro** del contenedor: compila Folly, ParlayLib y los dos binarios oficiales |
| [construir_extras.sh](construir_extras.sh) | Dentro del contenedor, después del anterior: compila `fijar_hilos.so`, `bench_basura` y el binario de latencia de Williams con `-mcx16` |
| [preparar.sh](preparar.sh) | Lo mismo sin Docker: baja el artefacto y las dependencias como carpeta local, sin instalar nada en el sistema |
| [correr_nativo.sh](correr_nativo.sh) | Mide sin Docker, con `equipo` (`bench.cpp`), `oficial` (binarios del artefacto), `extra` (más rondas de latencia y pares sobre `oficial`) o `alineado` (`bench.cpp` con `alignas(128)`) |
| [pendientes.sh](pendientes.sh) | Encadena `preparar.sh`, `oficial`, `extra`, `alineado` y `analizar.py`, para no medir dos cosas a la vez |
| [revisar_al_amanecer.sh](revisar_al_amanecer.sh) | Sin nadie delante: comprueba los datos, hace copia de seguridad, pide a un agente que siga `TRASPASO.md` y vigila que no toque datos ni código. Registro en `bin/revision.log` |
| [controles.sh](controles.sh) | Dos controles: la alineación de las pilas en `bench.cpp` y el mismo código dentro y fuera de Docker |
| [control_williams.sh](control_williams.sh) | Williams con `std::atomic` frente a `boost::atomic`, en pares de núcleos. Necesita haber corrido `controles.sh` antes |

### Medición y análisis

| Archivo | Para qué sirve |
|---|---|
| [baterias.py](baterias.py) | El motor. Lanza cada punto en su propio proceso, en orden aleatorio, y escribe CSV. Los experimentos son `fig5`, `fig6`, `afinidad`, `latafin`, `pares`, `basura` y `paridad` |
| [analizar.py](analizar.py) | Lee los CSV y escribe las gráficas y `tablas.md` |
| [paper_fig5.csv](paper_fig5.csv) | Valores de la figura 5 del paper, sacados de su gráfico vectorial |

### `herramientas/`

| Archivo | Para qué sirve |
|---|---|
| [fijar_hilos.c](herramientas/fijar_hilos.c) | Librería que se carga con `LD_PRELOAD` e intercepta `pthread_create` para fijar cada hilo a un núcleo, sin tocar el binario del paper |
| [latencia_nucleos.cpp](herramientas/latencia_nucleos.cpp) | Mide cuánto tarda una línea de caché en viajar entre cada par de núcleos: el mapa que decide qué grupos hacen de «socket» |
| [bench_equipo.cpp](herramientas/bench_equipo.cpp) | Línea de órdenes para `codigo/bench.cpp` sin modificarlo: lanza un solo punto, para poder fijarlo a núcleos desde fuera |
| [bench_basura.cpp](herramientas/bench_basura.cpp) | Mide cuántos objetos y bloques muertos siguen ocupando memoria. Su «límite» es una fórmula propia, no del paper |
| [cota_cas.cpp](herramientas/cota_cas.cpp) y [correr_cota_cas.sh](herramientas/correr_cota_cas.sh) | Comprueban la cota del paper (como mucho P intentos de `CAS` en la recarga) sobre el código real de los autores, con una copia instrumentada que se genera al vuelo |
| [controles.py](herramientas/controles.py) | El motor de `controles.sh`: mide las dos variantes de cada punto alternando y en orden aleatorio |
| [comprobar_final.py](herramientas/comprobar_final.py) | Comprueba, sin ninguna IA, que `pendientes.sh` terminó bien: registro, filas, códigos de salida y figuras |
| [medir_memoria.py](herramientas/medir_memoria.py) | Mide cuántos GB/s pasan de verdad por la controladora de memoria con n = 10⁶, y el techo de la máquina |
| [alinear_memoria.py](herramientas/alinear_memoria.py) | Mide la RAM con `bench.cpp` sin alinear y alineado, alternando, con n = 10⁵ y n = 10⁶: comprueba si alinear frena porque las pilas dejan de caber en la L3 |
| [contar_imc.c](herramientas/contar_imc.c) | Lee los contadores de la controladora de memoria con `perf_event_open`, porque este kernel no trae `perf` |
| [ancho_banda.cpp](herramientas/ancho_banda.cpp) | Mide el techo de la RAM leyendo y copiando vectores que no caben en caché. También sirve para validar `contar_imc` |
| [sanitizers_rivales.sh](herramientas/sanitizers_rivales.sh) | ASan, UBSan y TSan sobre los rivales, con `bench.cpp` y con el `bench_stack` oficial dentro de Docker |
| [extraer_fig5.py](herramientas/extraer_fig5.py) | Recupera los valores de la figura 5 del paper a partir de su gráfico vectorial |

## Carpetas que se generan y no se versionan

`artefacto/`, `artefacto_docker/` y `bin/` están en el `.gitignore`. Contienen el artefacto de Zenodo, que es código de terceros, y lo compilado. Se regeneran con los comandos de arriba.

## Cosas que conviene saber antes de ejecutar

- **Una medición a la vez, con la máquina libre.** Son pruebas de rendimiento: cualquier otro programa pesado las altera. El orden aleatorio reparte el efecto, pero no lo elimina.
- **No edites un `.sh` mientras corre.** Bash lee el guion sobre la marcha y puede ejecutar a medias la versión nueva.
- **`correr_docker.sh` usa `sudo docker`.** Si tu usuario ya puede usar Docker: `DOCKER=docker bash correr_docker.sh`.
- **Otra CPU:** las listas de núcleos de `baterias.py` (`CLUSTERS_E`, `NUCLEOS_P`, `PARES`) están escritas para el Core Ultra 9 285. Mide primero el mapa de latencias con `latencia_nucleos.cpp` y adáptalas.
- **No se toca `codigo/bench.cpp`** (regla del equipo) **ni `replica/informe/`**. Los problemas que se encontraron en ellos están en la tabla «Cosas que el equipo debería revisar» de la Parte 8 de EXPLICACION.md.
