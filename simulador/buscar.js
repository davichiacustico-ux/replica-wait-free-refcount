/* buscar.js — simulador de intercalados del algoritmo (como el de la Parte 12 de la explicación).
   Ejecuta miles de intercalados aleatorios paso atómico a paso atómico, comprueba los invariantes del paper
   después de cada paso y muestra el contraejemplo más corto de cada variante rota.
   Usa paper_motor.js (copia del motor de las animaciones).
   Uso: node buscar.js [ejecuciones]          (por defecto 20000 por variante) */
const path = require('path');
const M = require(path.join(__dirname, 'paper_motor.js'));
const N = +(process.argv[2] || 20000);

console.log(`Búsqueda de contraejemplos: ${N} ejecuciones por variante, 3 hilos, P = 3, C = 6, 3 operaciones por hilo (30 % store)\n`);
console.log('variante                         con fallo   tipo de fallo (veces)            contraejemplo más corto');
const cortos = {};
for (const v of ['paper', 'ingenua', 'umbral0', 'sinReserva', 'sinSalida']) {
  const r = M.buscar({ P: 3, C: 6, hilos: 3, ops: 3, variante: v }, N, 1);
  const tipos = Object.keys(r.tipos).map((k) => `${M.FALLOS[k]} (${r.tipos[k]})`).join(', ') || 'ninguno';
  console.log(`${M.VARIANTES[v].corto.padEnd(32)} ${String(r.conFallo).padStart(9)}   ${tipos.padEnd(32)} ${r.mejor ? r.mejor.paso + ' pasos' : '—'}`);
  if (r.mejor) cortos[v] = r.mejor;
}
const val = M.buscar({ P: 4, C: 8, hilos: 4, ops: 6, variante: 'paper' }, N, 2);
const tot = Object.values(val.intentos).reduce((s, x) => s + x, 0);
console.log(`\nValidación del algoritmo del paper con 4 hilos, P = 4, C = 8, 6 operaciones por hilo: ${val.ejecuciones} ejecuciones, ${val.pasos} pasos atómicos, ${val.conFallo} fallos`);
console.log('intentos de CAS por recarga: ' + Object.keys(val.intentos).map((k) => `${k} → ${(100 * val.intentos[k] / tot).toFixed(2)} %`).join('   ') + '   (cota teórica P = 4)');

const traza = (v) => {
  const b = cortos[v]; if (!b) return;
  console.log(`\nContraejemplo más corto — ${M.VARIANTES[v].nombre}`);
  console.log('paso  hilo    acción                                                       cartel      hojas');
  b.registros.forEach((x) => {
    const s = x.snap; const hojas = s.libros.map((L) => L.id + '=' + (L.vivo ? L.rc : '†')).join(' ');
    console.log(`${String(x.n).padStart(4)}  ${(x.hilo || '—').padEnd(7)} ${x.acc.padEnd(60)} ${(s.cartel.ptr ? `(${s.cartel.ptr}, ${s.cartel.peso})` : 'null').padEnd(11)} ${hojas}`);
  });
  console.log('→ ' + b.fallo.msg);
};
(process.argv[3] ? process.argv[3].split(',') : ['ingenua', 'sinReserva']).forEach(traza);
