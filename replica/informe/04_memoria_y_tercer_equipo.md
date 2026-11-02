# Memoria y réplica en un tercer equipo

Responsable: Diego. Ver ../../RESPONSABILIDADES.md.

> Pendiente. Estado al 2026-11-02: no hay medida de memoria ni resultados de otro equipo.

## Método

- Memoria: pico de RSS (`/usr/bin/time -v`) de `bin/bench` en cada panel y por implementación. Afirmación que se contrasta: el algoritmo no tiene sobrecoste de espacio (no hay reclamación diferida).
- Otro equipo: `cd codigo && bash ejecutar_pruebas.sh ../replica/resultados/<nombre>-<cpu>`, anotando CPU, núcleos y si es WSL o Linux nativo.

## Resultados

Sin datos. Si no se entregan, el informe dirá que la réplica se hizo en un solo equipo (i5-10400).

## Diferencias con el paper

El paper no publica una medida de memoria; su afirmación se basa en el diseño (sin reclamación diferida).
