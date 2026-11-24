# Comandos para ejecutar en vivo

Terminal de VS Code (perfil «Ubuntu (WSL)»), desde `exposicion_paper/codigo`. Los mismos comandos están en el
historial de bash de WSL: pulsa ↑ o `Ctrl+R` y escribe una palabra (`asan`, `boceto`, `bench`…). También están como
tareas: `Ctrl+Shift+P` → «Tasks: Run Task».

```bash
cd /mnt/c/Users/David/Documents/Semestres/2026B/cpd2/exposicion_paper/codigo
```

## 1. La carrera (AddressSanitizer), ~2 s

```bash
./bin/estres_asan ingenua 2>&1 | grep -E 'ERROR|freed by|SUMMARY' | head -4   # versión ingenua: heap-use-after-free
./bin/estres_asan autores 12 200000                                              # código de los autores: sin errores
```

## 2. ThreadSanitizer, ~5 s

```bash
./bin/estres_tsan autores 8 50000
```

## 3. Boceto y variantes rotas, ~3 s

```bash
./bin/boceto              # paper: 0 errores, intentos de CAS <= P
./bin/boceto_sin_salida   # sin salida temprana: supera la cota P
./bin/boceto_c_igual_p    # C = P (rompe R-1): errores de memoria
```

## 4. Rendimiento y latencia (versión corta), ~1 min

```bash
./bin/bench rendimiento 0.2 2>&1 >/dev/null | grep -E 'n=1 +u=1 '   # 1 pila, 1 % updates: Autores, Libstdc++, Williams, Mutex
./bin/bench latencia 20000 2>&1 >/dev/null | grep load              # latencia de load, p = 1 y p = 12
```

## 5. Simulador (Node), ~2 s

Node no está en WSL: ejecútalo en una terminal de Windows (perfil PowerShell) o con la tarea «09».

```powershell
node simulador/buscar.js 2000 ingenua,sinSalida
```

## Si hay que recompilar

```bash
bash ejecutar_pruebas.sh /tmp/demo   # todo (~4 min), sin tocar capturas/
```
