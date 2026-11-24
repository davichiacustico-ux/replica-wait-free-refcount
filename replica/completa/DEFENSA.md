# Cómo defender y mejorar la réplica

Esta guía es para el equipo que va a exponer la réplica y responder preguntas. Sirve para dos cosas:

1. **Defenderla:** qué críticas te pueden hacer, qué responder y, sobre todo, **qué conceder**. Un jurado se fía más de quien reconoce un límite que de quien lo esconde.
2. **Mejorarla:** qué hacer, y en qué orden, para que esas críticas pierdan fuerza.

El centro de la guía es un **tribunal simulado** ([sección 3](#3-el-tribunal-14-cargos-contra-la-réplica)): 14 críticas, cada una con su acusación, su defensa y un fallo. Antes hay un resumen de la réplica en tres tamaños, y después un plan de mejoras, un ensayo de preguntas y una chuleta de cifras.

Todas las cifras salen de [EXPLICACION.md](EXPLICACION.md), de `tablas.md` o de los CSV de `resultados/david-ultra9-285/`. **Si una cifra de aquí no coincide con ellos, manda la del documento**, y conviene corregir esta guía.

---

## Índice

1. [La réplica en tres tamaños](#1-la-réplica-en-tres-tamaños)
2. [Antes del debate: ¿réplica o reproducción?](#2-antes-del-debate-réplica-o-reproducción)
3. [El tribunal: 14 cargos contra la réplica](#3-el-tribunal-14-cargos-contra-la-réplica)
4. [La sentencia](#4-la-sentencia)
5. [Plan de mejoras, por orden](#5-plan-de-mejoras-por-orden)
6. [Ensayo: 20 preguntas del jurado](#6-ensayo-20-preguntas-del-jurado)
7. [Lo que no hay que decir, y qué decir en su lugar](#7-lo-que-no-hay-que-decir-y-qué-decir-en-su-lugar)
8. [Chuleta de cifras](#8-chuleta-de-cifras)
9. [Dónde está cada prueba](#9-dónde-está-cada-prueba)

---

## 1. La réplica en tres tamaños

Aprende las tres versiones. La corta es para cuando te interrumpen; la larga, para cuando te dejan hablar.

**En 20 segundos:**

> «Repetimos el paper con los programas oficiales de los autores en un PC de 24 núcleos, con y sin Docker: más de 14 000 mediciones y ninguna fallida. De las 15 afirmaciones del paper, 9 se reproducen, 2 en parte, 3 no y 1 no se confirma. Lo central sale bien: el algoritmo es correcto, cumple su cota *wait-free* y lee rápido. Lo que no sale es que gane a Folly.»

**En 1 minuto**, añade el *cómo*:

> «Usamos el artefacto publicado de los autores, con su suma MD5 comprobada, así que medimos su código y no una copia nuestra. El PC no tiene 4 procesadores como la máquina del paper, así que dividimos los hilos entre 6 y simulamos la distancia entre procesadores colocando cada hilo en núcleos concretos. Recortamos las mediciones de 10 a 2 segundos para que cupieran en una noche, y luego comprobamos con la receta exacta cuánto movía eso los números. También medimos si Docker influía: no. Y pasamos detectores de errores de memoria a todos los rivales, no solo al algoritmo del paper.»

**En 3 minutos**, cuenta la historia completa en este orden:

1. **El problema:** leer un puntero compartido y sumar 1 a su contador no es atómico. Entre las dos cosas, otro hilo puede liberar el objeto. Es la carrera *read-reclaim*.
2. **La idea del paper:** la lectura usa un solo `fetch_add`, que nunca falla, y por eso es *wait-free*.
3. **Lo que se reproduce:** gana a Libstdc++ y a Williams; Anderson gana con muchas escrituras, como dice el paper; no retiene memoria; la cota de reintentos se cumple con mucho margen.
4. **Lo que no se reproduce, y por qué no es un error nuestro:** Folly ya rinde distinto con **un solo hilo**, donde no hay pelea entre hilos; y con un millón de pilas todos chocan con el techo de la RAM de este PC.
5. **Lo que descubrimos por el camino:** el programa del equipo tenía compartición falsa, el binario oficial de latencia de Williams llevaba un candado escondido, y alinear las pilas frena con un millón de pilas porque dejan de caber en la caché.
6. **Los límites:** sin 4 sockets, sin NUMA, la latencia con contención es ruidosa y la RAM es de un solo módulo.

---

## 2. Antes del debate: ¿réplica o reproducción?

Un juez exigente puede empezar por aquí, así que conviene tenerlo claro.

La ACM, la asociación que pone las insignias de reproducibilidad en muchos congresos, [distingue tres palabras](https://www.acm.org/publications/policies/artifact-review-and-badging-current):

| Palabra | Quién lo hace | Con qué | ¿Es lo nuestro? |
|---|---|---|---|
| **Repetición** | El mismo equipo | Su propio montaje | No |
| **Reproducción** | Otro equipo | Los artefactos de los autores | **Sí**: programas oficiales del paper, en otra máquina |
| **Réplica** | Otro equipo | Artefactos hechos por él, desde cero | **En parte**: `bench.cpp` es un programa de pruebas del equipo, pero usa la implementación de los autores |

**Qué hacer con esto:** en el curso se le llama «réplica», y está bien. Pero si un juez dice «esto es una reproducción», **dale la razón**. No cuesta nada y demuestra que sabes de qué hablas. Y añade que, además, `bench.cpp` aporta una segunda opinión con un programa distinto.

---

## 3. El tribunal: 14 cargos contra la réplica

**Cómo funciona.** Hay tres voces, y las tres las hago yo:

- **Juez escéptico.** Busca el punto débil y aprieta. Es el jurado más duro que te puedes encontrar.
- **Juez defensor.** Responde solo con datos que estén en el repositorio.
- **Presidencia.** Escucha a los dos y dicta un fallo:
  - **A favor de la réplica**: la crítica no se sostiene.
  - **Concedido en parte**: la crítica tiene algo de razón, y hay que reconocerlo.
  - **En contra**: la crítica tiene razón, y hay que arreglarlo.

Cada cargo termina con **cómo blindarlo** (qué mejorar) y una **respuesta de 20 segundos** para decirla tal cual.

El reparto sugerido sigue a [RESPONSABILIDADES.md](../../RESPONSABILIDADES.md): corrección y latencia para David; rendimiento para Jheremy; memoria y comparación para Diego. Ajústalo a quien exponga cada parte.

| # | Cargo | Fallo | Quién lo defiende |
|---|---|---|---|
| 1 | «Sin 4 sockets no es una réplica» | Concedido en parte | Jheremy |
| 2 | «Vuestra simulación de sockets es un decorado» | A favor, con matiz | Jheremy |
| 3 | «Recortasteis las mediciones» | A favor, con matiz | Jheremy |
| 4 | «Folly no se reproduce porque lo hicisteis mal» | A favor; la pregunta sigue abierta | Jheremy |
| 5 | «La latencia es puro ruido» | Concedido en parte | David |
| 6 | «Docker contamina las medidas» | A favor | Diego |
| 7 | «Os equivocasteis tres veces: ¿por qué fiarse?» | A favor | Quien lo cuente |
| 8 | «"0 errores" no demuestra que sea correcto» | Concedido, y ya lo decís | David |
| 9 | «Con un millón de pilas solo medisteis vuestra RAM» | Concedido en parte | Diego |
| 10 | «Elegisteis lo que os convenía» | A favor | Cualquiera |
| 11 | «Sin intervalos de confianza no hay ciencia» | Concedido en parte | Jheremy |
| 12 | «La máquina no estaba controlada» | Concedido en parte | Cualquiera |
| 13 | «¿Esto lo hicisteis vosotros o una IA?» | Depende de vosotros | Todos |
| 14 | «Vuestro informe contradice vuestros datos» | **En contra, si no se corrige antes** | Cada responsable |

---

### Cargo 1. «Sin 4 sockets no es una réplica»

**Juez escéptico:** El paper mide la figura 5 en 4 Xeon E7-8867: 72 núcleos y 144 hilos, con la memoria repartida entre 4 chips. Vosotros tenéis un PC de escritorio con 24 núcleos, un solo chip y sin NUMA. Habéis medido otra cosa.

**Juez defensor:** Hemos medido **lo mismo, a otra escala**, y lo decimos. Tres argumentos:

1. **Se comparan algoritmos, no máquinas.** Esta CPU es casi diez años más moderna, así que los Mop/s absolutos no significan nada. Lo que se compara es el **orden** y la **proporción**: si Autores era el doble de rápido que un rival allí, ¿lo es también aquí? (Parte 6, regla 1.)
2. **La escala se conservó.** Los hilos del paper (1 a 216) se dividieron entre 6: 24 hilos es «toda la máquina», como 144 allí, y 36 es 1,5 veces, como 216 (Parte 4.3).
3. **Lo que depende de la escala está marcado.** Por ejemplo, en el paper Anderson solo adelanta a Autores con n = 1 y u = 1 % **a partir de 54 hilos**. Aquí no se llega a esa zona, y el documento lo explica como «una diferencia de alcance más que una contradicción» (Parte 6.1).

**Juez escéptico:** Entonces no podéis decir nada sobre NUMA ni sobre 4 sockets.

**Juez defensor:** Correcto, y no lo decimos. Está en la Parte 8, «No hay 4 sockets» y «No hay NUMA»: `numactl -i all` aquí no hace nada.

**Presidencia: concedido en parte.** La reproducción vale para comparar algoritmos a escala de un chip. Cualquier conclusión sobre 4 sockets o NUMA queda fuera, y el documento ya lo reconoce. No es un defecto escondido: es un límite declarado.

**Cómo blindarlo:**
- Lo ideal: repetir `correr_docker.sh` en una máquina de 2 o 4 sockets, sea un servidor de la universidad o una instancia *bare-metal* en la nube. Antes hay que medir su mapa con `herramientas/latencia_nucleos.cpp` y adaptar las listas de núcleos de `baterias.py`.
- Lo barato: repetir la batería de afinidad en un **AMD Ryzen con dos chiplets (CCD)**, si alguien del equipo o un conocido tiene uno. Pasar datos de un chiplet a otro cuesta varias veces más que dentro del mismo, así que el castigo por distancia se parecería más al de varios sockets. Hay que medirlo antes de darlo por hecho.

**En 20 segundos:** «Tienes razón: no tenemos 4 sockets ni NUMA, y por eso no sacamos conclusiones sobre eso. Comparamos algoritmos entre sí, con los hilos escalados para que 24 sea toda la máquina, igual que 144 en el paper.»

---

### Cargo 2. «Vuestra simulación de sockets es un decorado»

**Juez escéptico:** Decís que simuláis 4 Xeon con los 4 clústeres de núcleos E. Pero un clúster no es un socket.

**Juez defensor:** Por eso **descartamos el decorado de verdad**. Una máquina virtual con 4 nodos NUMA engaña al sistema operativo, pero ningún viaje tarda más (Parte 5.1). En su lugar:

1. **Medimos antes de suponer.** Una «pelota» entre cada par de núcleos dio 35 ns dentro de un clúster, 47 ns entre clústeres y 86 ns entre núcleos P (Parte 5.2). Las distancias existen y son físicas.
2. **Diseñamos un experimento de dosis-respuesta** con 4 hilos: lo único que cambia es la distancia. Al separarlos, Autores pierde entre un 32 % y un 54 % (Parte 6.4).
3. **Tiene un control.** Con n = 10⁶, donde no hay pelea por la misma línea de caché, separar no cambia nada: ±2 % con Docker, hasta +3 % sin Docker. Si el montaje estuviera mal, el castigo aparecería también ahí.

**Juez escéptico:** Pero vuestro castigo es de 1,35× y entre sockets reales ronda 3×.

**Juez defensor:** Sí, y además ese 3× es un dato indirecto, de máquinas parecidas a la del paper. Por eso el documento dice que la simulación «sirve para ver qué algoritmos son sensibles a la distancia, no para predecir los números de la máquina del paper». Es como un simulador de vuelo de juguete: la física es real, pero más suave.

**Presidencia: a favor de la réplica, con matiz.** El método es serio: medido, con control y con el límite dicho en voz alta. Lo que no se puede hacer es usarlo para predecir cifras de 4 sockets.

**Cómo blindarlo:** lo mismo que en el cargo 1. Además, enseña la figura `afinidad_dosis_respuesta.png` y señala el panel n = 10⁶ plano: es el mejor argumento visual.

**En 20 segundos:** «No usamos una máquina virtual, porque ahí nada tarda más. Medimos distancias reales, de 35 a 86 ns, y comprobamos con un control que el efecto solo aparece donde hay contención. Es más suave que 4 sockets reales, y lo decimos.»

---

### Cargo 3. «Recortasteis las mediciones»

**Juez escéptico:** El paper usa la mediana de 7 ejecuciones de 10 s. Vosotros, 5 de 2 s. Habéis medido una quinta parte.

**Juez defensor:** Lo hicimos para que la figura 5 pasara de unas 5 horas a poco más de una, y lo compensamos: cada punto va en un proceso nuevo y **en orden aleatorio**, para que el calor o una interrupción afecte a todos por igual (Parte 4.3). Después **comprobamos cuánto movía el recorte**, repitiendo con la receta exacta (Parte 6.7):

- Con Docker, en 2 escenarios: desviaciones de hasta un 21 %.
- Sin Docker, en los 6 escenarios (30 puntos): de −10 % a +19 %. La mitad, menos de un 6,7 %.
- **El orden se mantiene en 4 de 6.** En los otros dos, los que se cruzan están a menos de un 4 %, o en un empate que ya se conocía.

**Juez escéptico:** Un 19 % no es poco. Y encima decís que hay un sesgo.

**Juez defensor:** Lo decimos nosotros mismos: con contención, la versión corta da **más** en 23 de 25 puntos, y con n = 10⁶ da **menos** en los 5. Por eso la regla del documento es que **una diferencia pequeña no es una victoria**: solo cuentan las de 2 o 3 veces. Las conclusiones de la Parte 7 se apoyan en diferencias de ese tamaño, o se declaran empate.

**Presidencia: a favor de la réplica, con matiz.** El recorte está validado y sus consecuencias, aplicadas: la réplica no presume de diferencias pequeñas. El sesgo sin explicar es un punto débil real, pero declarado.

**Cómo blindarlo:** medir la figura 5 completa con la receta exacta, unas 5 horas, una noche. El experimento `paridad` de `baterias.py` ya mide con 10 s × 7; habría que ampliarlo a todos los hilos.

**En 20 segundos:** «Recortamos para que cupiera en una noche, y luego medimos el coste: el orden se mantiene en 4 de 6 escenarios, y en los otros 2 son empates. Por eso no damos por buena ninguna diferencia pequeña.»

---

### Cargo 4. «Folly no se reproduce porque lo hicisteis mal»

**Juez escéptico:** En el paper, con contención, Folly rinde entre 0,05× y 0,07× de Autores: veinte veces menos. A vosotros os sale entre 0,37× y 1,37×, y en un escenario gana. Lo más probable es que lo compilarais mal.

**Juez defensor:** Es la primera sospecha que tuvimos, y la comprobamos de cuatro formas:

1. **Es su Folly.** Sale del artefacto oficial de los autores (Zenodo, MD5 comprobado), compilado con su propio guion dentro de Docker.
2. **Compilado de dos formas, sale igual.** Dentro de Docker con `getdeps.py`, y fuera con CMake directo: mediana de 1,00× entre los dos.
3. **No es el recorte.** Con la receta exacta (7 × 10 s): de 0,36× a 1,26×.
4. **La prueba clave: con un solo hilo.** Ahí no hay pelea entre hilos ni nada viaja entre núcleos. En el paper, Folly ya rinde 0,20× con un hilo; aquí, 0,86×. En cambio, **Williams con un hilo coincide** con el paper (0,36× allí y 0,30× aquí, por ejemplo). El programa de pruebas funciona bien; lo distinto es Folly en la máquina del paper.

Y una pista del propio paper: en su figura 6, medida en otra máquina, el `load` de Folly sin contención tarda 24 ns, casi como Autores (23 ns). Folly no es lento de nacimiento.

**Juez escéptico:** Muy bien, pero no sabéis la causa.

**Juez defensor:** No, y no la inventamos. Hay candidatos: la generación del chip, o que la memoria estaba repartida entre 4 sockets incluso con un hilo. Pero con este PC no se puede saber, y el documento lo deja como pregunta abierta.

**Presidencia: a favor de la réplica.** La acusación era «lo hicisteis mal», y la defensa la desmonta con un argumento muy bueno: la brecha aparece con un hilo, mientras Williams coincide. Que la causa siga abierta no es un fallo de la réplica; es un hallazgo.

**Cómo blindarlo:**
- **Escribir a los autores** preguntando qué versión y configuración de Folly usaron en la figura 5. Cuesta un correo y puede cerrar la pregunta.
- Comprobar qué versión de Folly trae el artefacto y si coincide con la del paper.
- Medir la latencia de un solo `load` de Folly con un hilo, con contadores de hardware. Para eso habría que instalar `perf`, que este núcleo no trae.

**En 20 segundos:** «Lo primero que sospechamos fue un error nuestro, y lo descartamos. Es el Folly del artefacto, sale igual compilado de dos formas y con la receta exacta. Y la diferencia ya existe con un solo hilo, donde Williams sí coincide con el paper. La causa está en algo de su máquina que no podemos reproducir.»

---

### Cargo 5. «La latencia es puro ruido»

**Juez escéptico:** Con contención, vuestra latencia cambia muchísimo de una ronda a otra. Con 15 rondas y p = 24, la ronda más lenta casi sextuplica a la más rápida (5,75× de mediana, 36,9× en el peor caso). Con eso no se puede concluir nada.

**Juez defensor:** Con contención, poco, y lo reflejamos en el veredicto:

- **Subimos de 5 a 15 rondas** precisamente por eso (Parte 6.2).
- **Bajamos lo que no aguantó.** La fila 11 pasó de «Se reproduce» a «No se confirma»: la mejora se quedó en un 1-4 %, dentro del ruido. Las filas 7 y 9 se sostienen «por poco», y la 12, «en parte».
- **Tenemos una pista de dónde sale parte del ruido.** Con p = 24, el `load` de Autores sale en dos grupos, unos 420 ns y unos 1 450 ns. En las 3 rondas con los hilos fijados, el segundo grupo no apareció. Es un indicio de que depende de dónde coloca Linux al hilo medido, no una prueba.
- **p = 56 no se usa como comparación.** Con 56 hilos en 24 núcleos hay más hilos que núcleos, así que se mide, pero se declara no comparable.

Sin contención (p = 1), la latencia es estable: la ronda más alta y la más baja coinciden (1,00× en el caso típico y 1,1× en el peor, tabla «Variabilidad de la latencia entre rondas»). Ahí las filas 8 y 10 son claras.

**Presidencia: concedido en parte.** La latencia con contención es la parte más débil de la réplica, pero el documento lo trata bien: no presume de diferencias pequeñas y rebajó una fila cuando los datos lo pidieron.

**Cómo blindarlo:**
- Medir la figura 6 con la **frecuencia fija** (sin turbo), **núcleos aislados** del sistema y el **hilo medido siempre en el mismo núcleo**. Eso debería estrechar los rangos.
- Dar los percentiles con intervalos de confianza (ver cargo 11).

**En 20 segundos:** «Con contención, la latencia es muy variable. Por eso subimos a 15 rondas, y cuando una fila no aguantó la bajamos a "No se confirma". Sin contención, los resultados son estables.»

---

### Cargo 6. «Docker contamina las medidas»

**Juez escéptico:** Medisteis dentro de un contenedor. Eso añade capas, y no sabéis cuánto distorsiona.

**Juez defensor:** Sí lo sabemos, porque lo medimos: los mismos programas oficiales, dentro y fuera de Docker. **De 270 puntos de la figura 5, 259 cambian menos de un 10 %**, y 236, menos de un 5 %. Las medianas por algoritmo van de 0,99× a 1,01× (Parte 6.6).

**Juez escéptico:** Pero dentro y fuera cambian más cosas que Docker.

**Juez defensor:** Cierto: g++ 12.3 frente a 12.4, glibc 2.35 frente a 2.39, Boost 1.74 frente a 1.83, jemalloc 5.2.1 frente a 5.3.0 y la forma de compilar Folly. Por eso, cuando algo cambia, como Williams entre clústeres, **no lo achacamos a Docker sin más** (Parte 8).

**Presidencia: a favor de la réplica.** Medido, no supuesto, y con las salvedades bien dichas.

**Cómo blindarlo:** ya está blindado. Enseña la figura `docker_frente_a_nativo_oficial.png`: casi todos los puntos caen sobre la diagonal.

**En 20 segundos:** «Lo medimos: de 270 puntos, 259 cambian menos de un 10 % dentro y fuera de Docker.»

---

### Cargo 7. «Os equivocasteis tres veces: ¿por qué fiarse del resto?»

**Juez escéptico:** Vuestro propio documento confiesa tres errores. El programa del equipo, `bench.cpp`, tenía compartición falsa. Medisteis a Williams con un candado escondido. Y la primera medición de RAM dio lo contrario de la buena. ¿Cuántos más hay que no habéis visto?

**Juez defensor:** Los tres los encontramos nosotros, con controles diseñados para eso, y los tres están documentados:

1. **Compartición falsa en `bench.cpp`.** Sus 10 pilas cabían en 2 líneas de caché, y eso frenaba a Autores hasta 3,65 veces con n = 10. Lo destapó comparar con el programa oficial. Con las pilas alineadas, Autores pasa de 35 a 120 Mop/s, y la batería entera se repitió así (Parte 6.6).
2. **Williams con candado.** El CMake de latencia del artefacto compila sin `-mcx16`, y sin esa bandera la operación de 16 bytes lleva un candado escondido. Lo encontramos, recompilamos y lo comprobamos en el binario: 8 instrucciones `cmpxchg16b` y ninguna llamada al candado. Las medidas malas se conservan aparte, marcadas como descartadas (Parte 6.2).
3. **La RAM.** La primera versión medía mientras el programa liberaba memoria. Lo delató que el tráfico no crecía con el rendimiento (Parte 8).

Además, **el veredicto no se apoya en `bench.cpp`**, sino en los programas oficiales. `bench.cpp` es una segunda opinión.

**Juez escéptico:** Eso no responde a cuántos quedan.

**Juez defensor:** Nadie puede responder a eso. Lo que sí podemos decir es que buscamos errores activamente, y que cuando los encontramos los contamos.

**Presidencia: a favor de la réplica.** Un trabajo que documenta sus propios errores, con los datos descartados a la vista, merece **más** confianza, no menos. Pero el escéptico tiene un punto: la mejor defensa contra los errores que nadie ha visto es que otra persona lo repita.

**Cómo blindarlo:**
- Que **otro miembro del equipo** repita una parte en su propio PC. Ya estaba previsto en el reparto («¿se mantiene en otro equipo?»).
- Decidir si se corrige la compartición falsa de `codigo/bench.cpp`. Es una línea: `template <class I> struct alignas(128) Pila`.

**En 20 segundos:** «Los tres errores los encontramos nosotros con controles, y están documentados con los datos descartados a la vista. Además, el veredicto se apoya en los programas oficiales, no en el nuestro.»

---

### Cargo 8. «"0 errores" no demuestra que sea correcto»

**Juez escéptico:** Decís que el algoritmo es correcto porque ASan y TSan no encontraron nada. Una prueba solo ve lo que pasa mientras mira.

**Juez defensor:** Totalmente de acuerdo, y el documento lo dice con esas palabras: «encontrar 0 errores no demuestra que no existan». Lo que sí demuestran las pruebas es esto (Parte 6.5):

- **El detector funcionaba.** La versión ingenua lo hace saltar enseguida con un `double-free`. Y los binarios llevaban la instrumentación: se comprobó.
- **Las reglas del paper no son adornos.** Si se rompe a propósito la regla C ≥ 2P, aparecen 42 348 errores.
- **La cota *wait-free* se cumple con margen.** Forzando el caso extremo (P = 32), en 8 061 142 recargas el máximo fue de **8 intentos**, cuando la cota es 32.
- **Los rivales tampoco rompen la memoria:** 26 pruebas, 0 avisos.

La demostración de que el algoritmo es correcto está en el paper. Lo que nosotros comprobamos es que **su implementación** se comporta como dice.

**Juez escéptico:** Son 4 segundos por prueba y un solo escenario. Y Folly y Anderson no pasaron TSan.

**Juez defensor:** Correcto: `libfolly.a` no está instrumentada y daría avisos falsos. Está en la Parte 8.

**Presidencia: concedido, pero la réplica ya lo concede.** El veredicto dice «Se reproduce **en lo probado**». Esa frase es exactamente la correcta.

**Cómo blindarlo:**
- Pruebas de estrés más largas, de minutos u horas, y en más escenarios.
- TSan en Folly y Anderson, compilando Folly con `-fsanitize=thread`. Lleva tiempo de compilación.
- Si queréis subir de nivel, **comprobación de modelos** (por ejemplo, con GenMC) del núcleo del algoritmo con P y C pequeños. Explora todos los órdenes posibles entre hilos, no solo los que salen por azar.

**En 20 segundos:** «Tienes razón, una prueba no demuestra que sea correcto: eso lo hace la demostración del paper. Nosotros comprobamos que el detector funcionaba, que romper sus reglas produce errores y que la cota se cumple: 8 intentos como mucho frente a 32.»

---

### Cargo 9. «Con un millón de pilas solo medisteis vuestra RAM»

**Juez escéptico:** Con n = 10⁶, Autores, Anderson, Libstdc++ y Folly empatan entre 53 y 57 Mop/s, y el paper da a Autores como ganador. Vuestro PC tiene un solo módulo de RAM. La fila 5, «No se reproduce», es culpa de vuestra máquina.

**Juez defensor:** Probablemente sí, y por eso la fila 5 dice «No se reproduce **en este PC**». Pero no lo suponemos: **lo medimos** con los contadores de la controladora de memoria (Parte 6.1):

- El techo de este PC, leyendo líneas al azar, es de unos 37 GB/s.
- Con 24 hilos, Anderson llega al 93 % de ese techo, Libstdc++ al 92 % y Folly al 89 %.
- Al alinear las pilas, cada operación saca de la RAM entre 207 y 265 bytes más, y el rendimiento cae casi en la misma proporción (Parte 6.6). Cuando la RAM está al límite, más bytes por operación significa menos operaciones.

**Juez escéptico:** Pero Autores rinde lo mismo usando solo el 75 % del techo. Eso no lo explica la RAM.

**Juez defensor:** Correcto, y el documento lo dice: «no lo explica todo». Algo más frena a Autores; quizá el tiempo de espera de cada acceso, que no se midió. Por eso la conclusión es que la medición «refuerza» la explicación de la RAM, **no que la demuestre**.

**Presidencia: concedido en parte.** Es probable que la fila 5 dependa de la máquina, y el documento lo reconoce con cuidado. Falta la prueba definitiva.

**Cómo blindarlo:** **poner un segundo módulo de RAM** igual (DDR5-5600) y repetir `herramientas/medir_memoria.py` y el panel n = 10⁶. Si el rendimiento sube, queda demostrado. Es probablemente **la mejora con mejor relación entre coste y valor** de esta lista.

**En 20 segundos:** «Es muy probable, y por eso decimos "no se reproduce en este PC". Lo medimos: tres algoritmos llegan al 89-93 % del techo de la RAM. La prueba definitiva sería un segundo módulo de memoria.»

---

### Cargo 10. «Elegisteis lo que os convenía»

**Juez escéptico:** Es fácil reproducir un paper si solo enseñas lo que sale bien.

**Juez defensor:** Hicimos lo contrario:

- Los **6 escenarios** de la figura 5 y las **3 operaciones** de la figura 6, todos.
- Las **15 afirmaciones** del paper, una por una, incluidas las que no salen: 3 «No se reproduce» y 1 «No se confirma».
- Activamos a **Williams en la latencia**, que el paper omitió diciendo que «casi siempre era mayor». En vez de creer la frase, la comprobamos: sale «en parte».
- Los **datos descartados se conservan**, con la explicación de por qué se descartaron.
- Cada punto se midió en **orden aleatorio**, en un proceso nuevo.

**Presidencia: a favor de la réplica.** Es difícil acusar de selección a un trabajo que publica sus resultados negativos y sus propios errores.

**Cómo blindarlo:** en la exposición, dedica **una diapositiva a lo que no se reproduce**. Los jurados lo valoran mucho.

**En 20 segundos:** «Probamos las 15 afirmaciones, y 4 no salen; las contamos igual. Incluso medimos a Williams en latencia, que el paper había omitido.»

---

### Cargo 11. «Sin intervalos de confianza no hay ciencia»

**Juez escéptico:** Dais medianas y rangos de la peor a la mejor ronda. No hay intervalos de confianza ni pruebas estadísticas. ¿Cómo sabéis que una diferencia es real?

**Juez defensor:** Usamos una regla **conservadora**: si los rangos de las rondas se solapan, es empate. Y solo damos por buena una ventaja grande, de 2 o 3 veces, porque el recorte puede mover un punto hasta un 19 % (cargo 3). Para el orden general usamos la τ de Kendall, que siempre sale positiva: de +0,20 a +0,80 (Parte 6.1).

**Juez escéptico:** Conservadora, sí. Formal, no.

**Presidencia: concedido en parte.** La regla del solapamiento es prudente y está bien aplicada, pero un jurado de estadística pedirá algo más formal. Es fácil de añadir.

**Cómo blindarlo:** añadir a `analizar.py` **intervalos de confianza por *bootstrap*** para las medianas y los cocientes. Consiste en remuestrear las rondas miles de veces y ver cuánto se mueve la mediana. Son unas 30 líneas de Python. Con 5 rondas los intervalos saldrán anchos, y eso también es información honesta.

**En 20 segundos:** «Usamos una regla conservadora: si los rangos se solapan, es empate, y solo contamos ventajas de 2 o 3 veces. Añadir intervalos por bootstrap es una mejora que tenemos identificada.»

---

### Cargo 12. «La máquina no estaba controlada»

**Juez escéptico:** Es un PC de escritorio. No fijasteis la frecuencia, el turbo cambia solo, la CPU es híbrida y llegó a 99 °C.

**Juez defensor:** Todo eso es cierto y está en la Parte 8:

- **Temperatura:** solo 5 de las 3 720 ejecuciones con Docker y 26 de las 9 796 sin Docker llegaron a 90 °C o más. Están repartidas y se conservan.
- **Orden aleatorio:** cualquier interferencia afecta a todos los algoritmos por igual, en promedio.
- **CPU híbrida:** sin fijar los hilos, Linux los reparte entre núcleos P y E. Por eso la batería de afinidad sí los fija.

**Presidencia: concedido en parte.** El orden aleatorio reparte el efecto, pero no lo elimina. Las conclusiones grandes no dependen de esto; las diferencias pequeñas, quizá sí, y ya se tratan como empate.

**Cómo blindarlo:** gobernador `performance`, turbo desactivado y núcleos aislados del sistema. Luego, repetir un subconjunto alternando con la configuración normal, para ver si algo cambia.

**En 20 segundos:** «No fijamos la frecuencia, es cierto. Lo compensamos con el orden aleatorio y registramos la temperatura de cada ejecución: menos del 0,3 % llegó a 90 °C.»

---

### Cargo 13. «¿Esto lo hicisteis vosotros o una IA?»

**Juez escéptico:** El repositorio tiene decenas de guiones y un documento de 1 500 líneas, y en la Parte 8 se menciona «la sesión del asistente». ¿Lo entendéis, o lo ha hecho una máquina?

**Juez defensor:** Lo que da validez a una réplica no es quién tecleó cada línea, sino dos cosas:

1. **Que cualquiera pueda repetirla.** Cada experimento se lanza con una orden, que el README describe con su tiempo y su estado. Cada cifra sale de un CSV. Hay una comprobación automática sin IA, `herramientas/comprobar_final.py`.
2. **Que el equipo pueda explicarla.** Eso no lo da ninguna herramienta.

**Juez escéptico:** Lo primero lo cumplís. Lo segundo lo veremos con las preguntas.

**Presidencia: depende de vosotros.** Si el equipo puede explicar cada parte y ha declarado la ayuda según las normas del curso, no es un problema. Si no puede, este es **el cargo más peligroso de todos**, porque ningún dato lo defiende.

**Cómo blindarlo:**
- **Declarar el uso de herramientas** como pidan las normas del curso. Esconderlo es mucho peor que decirlo.
- Que **cada miembro repita una parte** con sus manos y sepa explicarla sin mirar. Por ejemplo, David la cota de `CAS`, Jheremy la figura 5 y Diego la memoria retenida.
- Ensayar la [sección 6](#6-ensayo-20-preguntas-del-jurado) en voz alta, con alguien que haga de jurado.

**En 20 segundos:** «Usamos herramientas, y lo declaramos. Pero todo se puede repetir con una orden y cada cifra sale de un CSV. Pregúntanos lo que quieras de cualquier parte.» Y después, que se note que lo sabéis.

---

### Cargo 14. «Vuestro informe contradice vuestros datos»

**Juez escéptico:** He leído vuestro informe. En `replica/informe/02_latencia.md` decís que el `load` de Libstdc++ con p = 56 tarda 86 700 ns, y la figura 6 del paper dice 99,7 µs. El README da 34,3 Mop/s en un sitio donde el CSV dice 39,126. Si no cuadran vuestros propios números, ¿por qué iba a creer el resto?

**Juez defensor:** Esos errores están localizados: la Parte 8 de EXPLICACION.md tiene una tabla, «Cosas que el equipo debería revisar». No se corrigieron porque esos archivos tienen responsables en el equipo.

**Juez escéptico:** Haberlos localizado no los arregla. El día de la exposición, el informe dirá lo que dice.

**Presidencia: en contra, si no se corrige antes.** Es el cargo más fácil de evitar y uno de los que más daño hace, porque un jurado que encuentra una errata empieza a desconfiar de todo. **Arreglarlo es cosa de horas.**

**Cómo blindarlo:** que cada responsable corrija su parte usando la tabla de la Parte 8:

| Dónde | Qué corregir |
|---|---|
| `replica/informe/02_latencia.md`, línea 18 | 86 700 ns → 99,7 µs, como en la figura 6 del paper |
| `README.md` de la raíz | Los números que no coinciden con `capturas/rendimiento.csv` |
| `codigo/ejecutar_pruebas.sh` | Añadir `setarch -R` al paso de TSan, como ya hace `correr_tsan.sh` |
| `codigo/bench.cpp`, línea 73 | Decidir si se alinea `Pila` con `alignas(128)` |
| Parte 7, fila 11 | Decidir si se queda en «No se confirma» o pasa a «Dudoso» |

**En 20 segundos:** solo vale una respuesta: «Ya está corregido».

---

## 4. La sentencia

**Presidencia:** Tras escuchar los 14 cargos, el tribunal resuelve así:

| Criterio | Valoración | Por qué |
|---|---|---|
| Artefactos disponibles | **Sí** | Todo está en el repositorio, con los CSV, las figuras y los guiones |
| Funcionan | **Sí** | Cada experimento es una orden, se reanuda solo, y la versión sin Docker se probó de principio a fin sin fallos |
| Reutilizables | **En parte** | Las listas de núcleos están escritas para el Core Ultra 9 285; en otra CPU hay que medir primero el mapa de latencias |
| Resultados reproducidos | **En parte, y bien argumentado** | 9 de 15 afirmaciones, más 2 en parte. Lo central del paper (correcto, *wait-free*, sin basura, lectura rápida) se sostiene |
| Transparencia | **Alta** | Errores propios documentados, datos descartados conservados, hipótesis marcadas como tales |
| Alcance | **Limitado, y declarado** | Nada sobre 4 sockets ni NUMA; la latencia con contención es débil; la RAM es de un solo módulo |

**Fallo final:** es una **reproducción válida y honesta a escala de un PC**. Confirma la idea central del paper y discute con datos lo que no reproduce. No sirve para hablar de 4 sockets ni de NUMA, y ella misma lo dice.

**Sus dos puntos más fuertes:** el argumento de Folly con un solo hilo (cargo 4) y haber medido en vez de suponer: Docker, los recortes, la RAM y la distancia (cargos 2, 3, 6 y 9).

**Sus dos puntos más débiles:** la latencia con contención (cargo 5) y, si no se corrigen antes, las erratas del informe del equipo (cargo 14).

---

## 5. Plan de mejoras, por orden

Ordenado por lo que más rinde para el tiempo que cuesta.

### Antes de la exposición (horas)

| Mejora | Cargo que debilita | Cómo |
|---|---|---|
| Corregir las erratas del informe y del README | 14 | Tabla del cargo 14 |
| Decidir la fila 11 y el `alignas(128)` de `bench.cpp` | 7 y 14 | Decisión del equipo |
| Declarar el uso de herramientas según las normas del curso | 13 | Una frase en el informe y en la exposición |
| Una diapositiva de «lo que no se reproduce y por qué» | 4 y 10 | Fila 3 y Folly con un hilo (Parte 6.1) |
| Diapositivas de reserva para las preguntas difíciles | Todos | Figuras: `fig5_paper_frente_a_replica.png`, `afinidad_dosis_respuesta.png`, `docker_frente_a_nativo_oficial.png`, `memoria_ram.png` y `nativo_alineado_fig5.png` |
| Ensayar la sección 6 en voz alta | 13 | Uno pregunta, otro responde, y se cambian los papeles |

### Si hay una semana

| Mejora | Cargo que debilita | Esfuerzo | Cómo empezar |
|---|---|---|---|
| Segundo módulo de RAM | 9 | Bajo, si alguien presta uno | Repetir `herramientas/medir_memoria.py` y el panel n = 10⁶ |
| Intervalos de confianza por *bootstrap* | 11, 3 y 5 | Bajo: una tarde | Una función en `analizar.py` que remuestree las rondas |
| Figura 5 completa con la receta exacta | 3 | Una noche, unas 5 h | Ampliar el experimento `paridad` de `baterias.py` a todos los hilos |
| Latencia con frecuencia fija y núcleos aislados | 5 y 12 | Medio: hay que reiniciar con otros parámetros del núcleo | Gobernador `performance`, turbo fuera, `isolcpus`, y repetir la figura 6 con 15 rondas |
| Otro miembro repite una parte en su PC | 7 y 13 | Medio | `correr_docker.sh` en otra máquina, adaptando las listas de núcleos |
| Williams con Boost 1.74 fuera de Docker | Pregunta abierta | Bajo o medio | Compilar `bench_stack` fuera con Boost 1.74, o dentro con 1.83 |
| Escribir a los autores sobre Folly | 4 | Muy bajo: un correo | Preguntar versión y configuración de Folly en la figura 5 |

### Si queréis llevarlo más lejos

| Mejora | Cargo que debilita | Esfuerzo |
|---|---|---|
| Máquina de 2 o 4 sockets (servidor de la universidad o nube) | 1 y 2 | Alto: acceso o dinero |
| Batería de afinidad en un AMD Ryzen con dos chiplets | 1 y 2 | Medio, si hay acceso a uno |
| TSan en Folly y Anderson | 8 | Alto: hay que compilar Folly instrumentado |
| Pruebas de estrés largas y en más escenarios | 8 | Bajo de hacer, largo de esperar |
| Comprobación de modelos (por ejemplo, GenMC) con P y C pequeños | 8 | Alto: hay que aprender la herramienta |
| Contadores de hardware (`perf`) para Folly y para Autores con pocos hilos | 4 y 9 | Medio: instalar `perf` para este núcleo |

---

## 6. Ensayo: 20 preguntas del jurado

Respuestas cortas. Si piden más, la sección indicada tiene el detalle.

1. **¿Por qué no una máquina virtual con 4 nodos NUMA?** Porque sería un decorado: el sistema creería que hay 4 nodos, pero ningún viaje tardaría más. Preferimos distancias reales y medidas. (Parte 5.1)
2. **¿Por qué dividisteis los hilos entre 6?** Para conservar la proporción: 144 hilos eran toda la máquina del paper y 24 son toda esta. (Parte 4.3)
3. **¿Por qué 2 s y no 10?** Para que la figura 5 cupiera en una hora en vez de cinco. Después medimos cuánto cambiaba: el orden se mantiene en 4 de 6 escenarios. (Parte 6.7)
4. **¿Por qué orden aleatorio y un proceso por punto?** Para que el calor o una interrupción afecten a todos por igual, y que ningún punto herede el estado del anterior. (Parte 4.3)
5. **¿Usasteis el código de los autores?** Sí: el artefacto de Zenodo (`10.5281/zenodo.11775182`), con el MD5 comprobado. `bench.cpp` del equipo es una segunda opinión. (Parte 4.2)
6. **¿Qué significa *wait-free* y cómo lo comprobasteis?** Que cada operación termina en un número acotado de pasos, haga lo que haga el resto. Contamos los intentos en una copia del código: 8 como mucho, con una cota de 32. (Parte 6.5)
7. **¿Por qué Folly sale tan distinto?** No lo sabemos. Sí sabemos que no es contención, porque la diferencia ya existe con un hilo, y que no es nuestro montaje, porque Williams sí coincide. (Parte 6.1)
8. **¿Por qué empatan todos con un millón de pilas?** Muy probablemente por la RAM: tres algoritmos llegan al 89-93 % del techo de este PC, que tiene un solo módulo. (Parte 6.1)
9. **¿Qué aporta vuestra réplica que no esté en el paper?** Williams en latencia, que el paper omitió; el candado escondido por compilar sin `-mcx16`; la batería de afinidad; y la medición real de la RAM. (Partes 6.2, 6.4 y 6.1)
10. **¿Influye Docker?** No: de 270 puntos, 259 cambian menos de un 10 %. (Parte 6.6)
11. **¿Cómo sabéis que ASan funcionaba?** Porque la versión ingenua lo hace saltar enseguida, y comprobamos que los binarios estaban instrumentados. (Parte 6.5)
12. **¿Qué es la compartición falsa?** Dos personas que escriben en renglones distintos de la misma hoja y tienen que pasarse el cuaderno cada vez. Datos distintos, misma línea de caché. (Parte 6.6)
13. **¿Por qué alinear frena con un millón de pilas?** Porque cada pila pasa de 8-16 bytes a 128, y el millón deja de caber en la caché L3. Medido: cada operación saca de la RAM entre 207 y 265 bytes más. (Parte 6.6)
14. **¿Por qué Williams se hunde entre clústeres?** No lo sabemos. Descartamos un par de núcleos defectuoso, Docker y `boost::atomic`. Solo pasa con el programa oficial. (Parte 6.4)
15. **¿Qué pasa con más hilos que núcleos?** Se mide (30 y 36 hilos, como 180 y 216 en el paper), pero hay que leerlo con cuidado. En latencia, p = 56 se declara no comparable. (Partes 4.3 y 6.1)
16. **¿Por qué Anderson gana cuando hay escrituras?** Porque en Autores cada lectura modifica el puntero compartido, y eso hace fallar los `CAS` de quien quiere escribir. En Anderson, leer no lo toca. (Parte 6.1)
17. **¿Se puede repetir?** Sí: cada experimento es una orden y se reanuda solo. Están en el README de `replica/completa/`, con lo que tarda cada uno. (Parte 9)
18. **¿Cuánta memoria retiene Anderson?** Hasta 19 960 bloques (469 KB) con 24 hilos. Autores, nada por encima del límite. (Parte 6.3)
19. **Con una semana más, ¿qué haríais?** Un segundo módulo de RAM, intervalos de confianza y la figura 5 con la receta exacta. (Sección 5)
20. **Vuestra conclusión, en una frase.** «La idea central del paper sale bien parada: es correcto, cumple su cota, no acumula basura y lee rápido. No se sostiene la superioridad frente a Folly ni la ventaja con muchísimas pilas, y la escala de 4 sockets no se pudo reproducir.» (Parte 7)

---

## 7. Lo que no hay que decir, y qué decir en su lugar

Muchas críticas nacen de una palabra demasiado fuerte. Cambia la palabra y la crítica desaparece.

| No digas | Di | Por qué |
|---|---|---|
| «Demostramos que es correcto» | «No encontramos errores buscándolos, y la cota se cumplió con margen» | Una prueba nunca demuestra corrección (cargo 8) |
| «Simulamos 4 Xeon» | «Simulamos la distancia entre sockets, de forma más suave; NUMA no» | 1,35× frente a unas 3× (cargo 2) |
| «El paper se equivoca con Folly» | «En nuestra máquina Folly rinde mucho mejor, ya con un hilo; la causa está abierta» | No tenemos su máquina (cargo 4) |
| «Docker no influye» | «Lo medimos: 259 de 270 puntos cambian menos de un 10 %» | La cifra convence más que el adjetivo (cargo 6) |
| «La RAM es el techo» | «Tres algoritmos llegan al 89-93 % del techo de la RAM; lo refuerza, no lo demuestra» | Falta el segundo módulo (cargo 9) |
| «Autores gana» por un 5 % | «Empatan» | Las diferencias pequeñas están dentro del ruido (cargos 3 y 11) |
| «Es una réplica exacta» | «Es una reproducción con los programas oficiales, a otra escala» | Terminología (sección 2) |
| «Eso es culpa de nuestro PC» | «Eso depende de la máquina, y lo marcamos como "en este PC"» | Suena a excusa; lo otro, a método |

---

## 8. Chuleta de cifras

| Qué | Cifra |
|---|---|
| Mediciones | Más de 14 000, ninguna fallida |
| Veredicto de las 15 afirmaciones | 9 se reproducen, 2 en parte, 3 no y 1 no se confirma |
| Máquina | Core Ultra 9 285: 24 núcleos (8 P + 16 E), sin SMT, 1 nodo NUMA, un módulo de 32 GB DDR5-5600 |
| Máquina del paper (figura 5) | 4 Xeon E7-8867: 72 núcleos, 144 hilos |
| Hilos | Los del paper divididos entre 6: 24 aquí equivale a 144 allí |
| Cota *wait-free* | Máximo 8 intentos con P = 32, en 8 061 142 recargas |
| Romper la regla C ≥ 2P | 42 348 errores |
| Detectores en los rivales | 26 pruebas, 0 avisos |
| Mejor caso de Autores (n = 10, u = 1 %, 24 hilos) | Autores 122, Anderson 74, Folly 46, Williams 35 y Libstdc++ 11 Mop/s |
| Anderson con muchas escrituras (n = 1) | 1,87× a 1,99× de Autores |
| Folly con un hilo | 0,20× en el paper frente a 0,86× aquí (n = 1, u = 1 %) |
| Docker frente a sin Docker | 259 de 270 puntos cambian menos de un 10 % |
| Recortes, en los 6 escenarios | De −10 % a +19 %; el orden se mantiene en 4 de 6 |
| Latencias entre núcleos | 35 ns en el mismo clúster, 47 entre clústeres, 86 entre núcleos P |
| Separar 4 hilos | Autores pierde un 32-54 % con contención; con n = 10⁶, ±2 % |
| Techo de la RAM | 43 GB/s en orden, 37 al azar |
| Memoria retenida por Anderson | Hasta 19 960 bloques, 469 KB, con 24 hilos |
| Compartición falsa en `bench.cpp` | Autores, de 35 a 120 Mop/s al alinear (n = 10) |
| Alinear con n = 10⁶ | −24 a −27 %, con 207-265 bytes más de RAM por operación |
| Williams entre clústeres | −97 % con Docker, −76 % sin Docker |

---

## 9. Dónde está cada prueba

Si alguien pide ver los datos, aquí está el camino. Las tablas están en `resultados/david-ultra9-285/tablas.md` y las figuras, en `resultados/david-ultra9-285/figuras/`.

| Para defender… | Sección de EXPLICACION.md | Figura |
|---|---|---|
| El veredicto completo | [Parte 7](EXPLICACION.md#parte-7-el-veredicto-afirmación-por-afirmación) | — |
| Los límites | [Parte 8](EXPLICACION.md#parte-8-limitaciones-dicho-sin-rodeos) | — |
| Las adaptaciones (hilos, recortes) | [Parte 4.3](EXPLICACION.md#43-adaptaciones-y-por-qué) | — |
| La simulación de sockets | [Parte 5](EXPLICACION.md#parte-5-simular-4-xeon-con-una-sola-cpu) y [6.4](EXPLICACION.md#64-la-simulación-de-los-4-xeon-importa-la-distancia) | `topologia.png`, `afinidad_dosis_respuesta.png` |
| Rendimiento, Folly y la RAM | [Parte 6.1](EXPLICACION.md#61-rendimiento-la-figura-5) | `fig5_paper_frente_a_replica.png`, `memoria_ram.png` |
| Latencia y su ruido | [Parte 6.2](EXPLICACION.md#62-latencia-la-figura-6) | `fig6_replica.png`, `nativo_oficial_fig6.png` |
| Memoria retenida | [Parte 6.3](EXPLICACION.md#63-memoria-quién-deja-basura) | `memoria_retenida.png` |
| Corrección y cota | [Parte 6.5](EXPLICACION.md#65-corrección-funciona-sin-romperse) | — |
| Docker, compartición falsa y alineado | [Parte 6.6](EXPLICACION.md#66-la-versión-sin-docker) | `docker_frente_a_nativo_oficial.png`, `nativo_alineado_fig5.png` |
| Los recortes | [Parte 6.7](EXPLICACION.md#67-validación-de-los-recortes) | — |
| Cómo repetirlo | [Parte 9](EXPLICACION.md#parte-9-cómo-repetirlo-todo) y [README.md](README.md) | — |
