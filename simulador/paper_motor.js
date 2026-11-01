/* paper_motor.js — modelo paso a paso del algoritmo de de Wolff, Anderson, Keller y Seletskiy (Euro-Par 2024).
   Cada instrucción atómica es un paso: el fetch_and_add de load, el swap de store, cada CAS y cada suma o resta en el
   contador (la «hoja») de un objeto (un «libro»). Entre dos pasos de un hilo puede ejecutarse cualquier otro hilo.
   Cada hilo es una pequeña máquina de estados: así se puede mostrar qué hará en su próximo paso sin ejecutarlo.
   Después de cada paso se comprueban las propiedades que el paper demuestra y las que rompen las variantes erróneas.
   No depende del DOM: se prueba con Node (pruebas/test_paper.js). */
(function (root) {
  'use strict';

  const VARIANTES = {
    paper: { nombre: 'El algoritmo del paper', corto: 'Paper', que: 'load con un solo fetch_and_add; recarga con reserva previa y salida temprana.' },
    ingenua: { nombre: 'Versión ingenua: leer y después apuntarse', corto: 'Ingenua', que: 'Sin pesos: load lee el puntero y, en otro paso, suma 1 al contador.' },
    umbral0: { nombre: 'Recargar solo cuando el talonario llega a 0', corto: 'Umbral 0', que: 'load llama a increase_weight solo si el peso queda en 0, no al bajar de P.' },
    sinReserva: { nombre: 'Recargar el cartel antes de apuntar la reserva', corto: 'Sin reserva previa', que: 'increase_weight hace primero el CAS y después suma C − w a la hoja.' },
    sinSalida: { nombre: 'Recarga sin la salida temprana de R-2', corto: 'Sin salida temprana', que: 'Si el CAS falla con el mismo libro, reintenta siempre, aunque el peso haya subido.' },
  };
  const FALLOS = {
    uaf: 'uso tras liberar', prematura: 'liberación prematura', negativo: 'peso negativo', desborde: 'desborde',
    respaldo: 'respaldo', inv1: 'invariante 1', inv2: 'invariante 2', fuga: 'fuga',
  };

  /* generador pseudoaleatorio reproducible (mulberry32) */
  function rng(semilla) {
    let a = (semilla >>> 0) || 1;
    return () => { a = (a + 0x6D2B79F5) >>> 0; let t = a; t = Math.imul(t ^ (t >>> 15), t | 1); t ^= t + Math.imul(t ^ (t >>> 7), t | 61); return ((t ^ (t >>> 14)) >>> 0) / 4294967296; };
  }
  const clon = (x) => JSON.parse(JSON.stringify(x));
  const norm = (o) => (typeof o === 'string' ? { op: o } : Object.assign({}, o));

  class Sim {
    /* cfg: { P, C, variante, cartel: { libro, peso } | null, libros: { id: rc }, hilos: [{ nombre, ops, pases }], nuevos: [nombres] }
       ops: 'load' | 'usar' | 'soltar' | 'store' | { op: 'store', libro, conservar } */
    constructor(cfg) {
      this.cfg = cfg; this.P = cfg.P; this.C = cfg.C; this.var = cfg.variante || 'paper';
      this.libros = {}; this.orden = [];
      this.hilos = cfg.hilos.map((h, i) => ({ i, nombre: h.nombre, ops: (h.ops || []).map(norm), k: 0, pc: null, loc: {}, pases: (h.pases || []).slice(), linea: '', hecho: false }));
      const extra = {};
      this.hilos.forEach((h) => h.pases.forEach((p) => { extra[p] = (extra[p] || 0) + 1; }));
      const c = cfg.cartel;
      this.cartel = c ? { ptr: c.libro, peso: c.peso } : { ptr: null, peso: 0 };
      const ids = [];
      [...(c ? [c.libro] : []), ...Object.keys(cfg.libros || {}), ...Object.keys(extra)].forEach((x) => { if (!ids.includes(x)) ids.push(x); });
      ids.forEach((id) => { const rc = cfg.libros && id in cfg.libros ? cfg.libros[id] : (c && c.libro === id ? c.peso : 0) + (extra[id] || 0); this.nuevoLibro(id, rc); });
      this.nuevos = (cfg.nuevos || []).slice(); this.nNuevo = 1 + ids.length;
      this.O = []; this.fallos = []; this.n = 0; this.intentos = []; this.cerrado = false; this._nuevos = [];
      this.hilos.forEach((h) => this.empezar(h));
      this.registros = [{ n: 0, hilo: null, hi: -1, acc: 'estado inicial', linea: '', nuevos: [], snap: this.snap() }];
    }
    nuevoLibro(id, rc) { this.libros[id] = { id, rc, vivo: true }; this.orden.push(id); }
    nombreNuevo() { if (this.nuevos.length) return this.nuevos.shift(); let id; do { id = 'L' + this.nNuevo++; } while (this.libros[id]); return id; }

    /* ---------- comprobaciones ---------- */
    fallo(tipo, msg) { if (!this.fallos.some((f) => f.tipo === tipo)) { const f = { tipo, msg, paso: this.n }; this.fallos.push(f); this._nuevos.push(f); } }
    tocar(h, id) { const L = this.libros[id]; if (L && !L.vivo) this.fallo('uaf', `USO TRAS LIBERAR: ${h.nombre} toca ${id}, que ya estaba liberado.`); }
    pasesDe(id) { return this.hilos.reduce((s, h) => s + h.pases.filter((x) => x === id).length, 0); }
    refs(id) { return (this.cartel.ptr === id ? Math.max(0, this.cartel.peso) : 0) + this.pasesDe(id); }
    restar(id, n) {   // resta n a la hoja; si llega a 0 (o menos) el libro se libera en ese momento
      const L = this.libros[id]; if (!L) return false;
      L.rc -= n;
      if (L.rc <= 0 && L.vivo) {
        L.vivo = false; const r = this.refs(id);
        if (r > 0) this.fallo('prematura', `LIBERACIÓN PREMATURA: ${id} se liberó con ${r} ${r === 1 ? 'referencia' : 'referencias'} todavía en uso.`);
        return true;
      }
      return false;
    }
    respaldo() {   // por libro vivo: hoja ≥ tickets del cartel + pases
      return this.orden.filter((id) => this.libros[id].vivo).map((id) => {
        const t = this.cartel.ptr === id ? Math.max(0, this.cartel.peso) : 0; const p = this.pasesDe(id);
        return { id, rc: this.libros[id].rc, tickets: t, pases: p, ok: this.libros[id].rc >= t + p };
      });
    }
    comprobar() {
      const c = this.cartel;
      if (c.ptr && c.peso < 0) this.fallo('negativo', `PESO NEGATIVO: el cartel marca ${c.peso} tickets.`);
      if (c.ptr && this.var !== 'ingenua' && c.peso > this.C) this.fallo('desborde', `DESBORDE: el cartel marca ${c.peso} tickets, más que C = ${this.C}.`);
      if (this.var === 'paper') {
        this.respaldo().forEach((r) => { if (!r.ok) this.fallo('respaldo', `RESPALDO: la hoja de ${r.id} marca ${r.rc}, menos que ${r.tickets} tickets + ${r.pases} pases.`); });
        const vistos = {}; this.O.forEach((x) => { vistos[x] = (vistos[x] || 0) + 1; });
        Object.keys(vistos).forEach((x) => { if (vistos[x] > 1) this.fallo('inv1', `INVARIANTE 1: ${x} está ${vistos[x]} veces en O.`); });
        if (c.ptr && c.peso < this.P - this.O.length) this.fallo('inv2', `INVARIANTE 2: peso ${c.peso} < P − |O| = ${this.P - this.O.length}.`);
      }
    }

    /* ---------- máquina de estados de cada hilo ---------- */
    empezar(h) {   // coloca el contador de programa al principio de la operación k
      const o = h.ops[h.k];
      h.loc = {};
      if (!o) { h.pc = null; h.hecho = true; return; }
      h.pc = { load: this.var === 'ingenua' ? 'ing.lee' : 'load.faa', store: 'store.crea', usar: 'usar', soltar: 'soltar' }[o.op];
      if (!h.pc) throw new Error('operación desconocida ' + o.op);
    }
    terminar(h) { h.k++; this.empezar(h); }
    /* qué hará el hilo en su próximo paso (texto) */
    prox(h) {
      const L = h.loc; const o = h.ops[h.k] || {};
      switch (h.pc) {
        case null: case undefined: return '';
        case 'ing.lee': return 'load: lee el cartel';
        case 'ing.inc': return L.p ? `apunta +1 en la hoja de ${L.p}` : 'termina';
        case 'load.faa': return 'load: fetch_and_add (arranca un ticket)';
        case 'iw.res': return `reserva +${L.d} en la hoja de ${L.p}`;
        case 'iw.cas': return `CAS ${L.cur}→${L.cur + L.d}`;
        case 'iw.apunta': return `apunta +${L.d} en la hoja de ${L.p}`;
        case 'iw.dev': return `devuelve la reserva: −${L.d} en la hoja de ${L.p}`;
        case 'store.crea': return `store: crea ${o.libro || 'un libro nuevo'}`;
        case 'store.inc': return `apunta +${L.Cn} en la hoja de ${L.id}`;
        case 'store.swap': return `swap: cartel ${this.cartel.ptr || 'null'}→${L.id}`;
        case 'store.dec': return `resta ${L.viejo.peso} a la hoja de ${L.viejo.ptr}`;
        case 'store.suelta': return `suelta su propio pase de ${L.id}`;
        case 'usar': return 'usa su libro';
        case 'soltar': return 'suelta su pase';
        default: return h.pc;
      }
    }
    /* ejecuta un paso atómico del hilo h; devuelve el texto de la acción */
    exec(h) {
      const L = h.loc; const c = this.cartel; const o = h.ops[h.k] || {};
      h.linea = h.pc;
      switch (h.pc) {
        case 'ing.lee': {
          L.p = c.ptr; h.pc = c.ptr ? 'ing.inc' : null;
          if (!c.ptr) this.terminar(h);
          return `lee el cartel: ${L.p || 'null'} (aún sin apuntarse)`;
        }
        case 'ing.inc': {
          this.tocar(h, L.p); const vivo = this.libros[L.p].vivo; this.libros[L.p].rc += 1; h.pases.push(L.p); const p = L.p; this.terminar(h);
          return `apunta +1 en la hoja de ${p}` + (vivo ? '' : ' ✗');
        }
        case 'load.faa': {
          const ptr = c.ptr; const peso = c.peso;
          if (!ptr) { this.terminar(h); return 'load: el cartel está vacío (null)'; }
          c.peso -= 1; h.pases.push(ptr);
          const ofensa = peso <= this.P; if (ofensa) this.O.push(h.nombre);
          const w = peso - 1; const umbral = this.var === 'umbral0' ? 1 : this.P;
          const txt = `load: arranca 1 ticket (${peso}→${w})` + (ofensa ? ', ofensa' : '');
          if (w < umbral) {
            Object.assign(L, { p: ptr, w, d: this.C - w, cur: w, intentos: 0, recargando: true });
            h.pc = this.var === 'sinReserva' ? 'iw.cas' : 'iw.res';
            return txt + '; le toca recargar';
          }
          this.terminar(h); return txt;
        }
        case 'iw.res': {
          this.tocar(h, L.p); this.libros[L.p].rc += L.d; L.reservado = true; h.pc = 'iw.cas';
          return `reserva +${L.d} en la hoja de ${L.p}`;
        }
        case 'iw.cas': {
          L.intentos++;
          const esperado = L.cur;
          if (c.ptr === L.p && c.peso === esperado) {
            c.peso = esperado + L.d; if (c.peso > this.P) this.O = [];
            this.intentos.push(L.intentos); L.exito = true;
            if (this.var === 'sinReserva') h.pc = 'iw.apunta'; else this.terminar(h);
            return `CAS ${esperado}→${esperado + L.d}: éxito`;
          }
          const np = c.ptr; const nw = c.peso; const visto = `(${np || 'null'}, ${nw})`;
          const salir = np !== L.p || (this.var !== 'sinSalida' && nw > esperado);
          if (salir) {
            this.intentos.push(L.intentos);
            const motivo = np !== L.p ? 'el cartel muestra otro libro' : `${nw} > ${esperado}: alguien recargó`;
            if (this.var === 'sinReserva') this.terminar(h); else h.pc = 'iw.dev';
            return `CAS esperaba ${esperado}, ve ${visto}: falla → sale (${motivo})`;
          }
          L.cur = nw;
          return `CAS esperaba ${esperado}, ve ${visto}: falla → reintenta con ${nw}`;
        }
        case 'iw.apunta': {
          this.tocar(h, L.p); this.libros[L.p].rc += L.d; const t = `apunta +${L.d} en la hoja de ${L.p}`; this.terminar(h); return t;
        }
        case 'iw.dev': {
          this.tocar(h, L.p); const lib = this.restar(L.p, L.d); const t = `devuelve la reserva: −${L.d} en la hoja de ${L.p}` + (lib ? ` → ${L.p} liberado` : ''); this.terminar(h); return t;
        }
        case 'store.crea': {
          L.id = o.libro || this.nombreNuevo(); L.Cn = this.var === 'ingenua' ? 1 : this.C;
          this.nuevoLibro(L.id, 1); h.pases.push(L.id); h.pc = 'store.inc';
          return `crea ${L.id} (hoja 1: su propio pase)`;
        }
        case 'store.inc': {
          this.tocar(h, L.id); this.libros[L.id].rc += L.Cn; h.pc = 'store.swap';
          return `apunta +${L.Cn} en la hoja de ${L.id}`;
        }
        case 'store.swap': {
          L.viejo = { ptr: c.ptr, peso: c.peso };
          this.cartel = { ptr: L.id, peso: L.Cn }; if (this.var !== 'ingenua' && L.Cn > this.P) this.O = [];
          h.pc = L.viejo.ptr ? 'store.dec' : (o.conservar ? null : 'store.suelta');
          if (!h.pc) this.terminar(h);
          const s = L.viejo.peso === 1 ? 'sobrante' : 'sobrantes';
          return `swap: cartel ${L.viejo.ptr || 'null'}→${L.id}` + (L.viejo.ptr ? `, se lleva ${L.viejo.peso} ${s}` : '');
        }
        case 'store.dec': {
          this.tocar(h, L.viejo.ptr); const lib = this.restar(L.viejo.ptr, L.viejo.peso);
          const t = `resta ${L.viejo.peso} a la hoja de ${L.viejo.ptr}` + (lib ? ` → ${L.viejo.ptr} liberado` : '');
          if (o.conservar) this.terminar(h); else h.pc = 'store.suelta';
          return t;
        }
        case 'store.suelta': {
          const k = h.pases.lastIndexOf(L.id); if (k >= 0) h.pases.splice(k, 1);
          this.tocar(h, L.id); const lib = this.restar(L.id, 1); const t = `suelta su propio pase de ${L.id}` + (lib ? ` → ${L.id} liberado` : '');
          this.terminar(h); return t;
        }
        case 'usar': {
          const id = h.pases[h.pases.length - 1]; this.terminar(h);
          if (!id) return 'no tiene libro que usar';
          this.tocar(h, id); return `usa ${id}` + (this.libros[id].vivo ? '' : ' ✗');
        }
        case 'soltar': {
          const id = h.pases.pop(); this.terminar(h);
          if (!id) return 'no tiene pases que soltar';
          this.tocar(h, id); const lib = this.restar(id, 1);
          return `suelta su pase de ${id}` + (lib ? ` → ${id} liberado` : '');
        }
        default: throw new Error('estado desconocido ' + h.pc);
      }
    }

    /* ---------- ejecución ---------- */
    listos() { return this.hilos.filter((h) => !h.hecho).map((h) => h.i); }
    terminado() { return this.hilos.every((h) => h.hecho); }
    paso(i) {
      const h = this.hilos[i]; if (!h || h.hecho) return null;
      this._nuevos = []; this.n++;
      const acc = this.exec(h); const linea = h.linea;
      this.comprobar();
      const rec = { n: this.n, hilo: h.nombre, hi: i, acc, linea, nuevos: this._nuevos.slice(), snap: this.cfg.ligero ? null : this.snap() };
      this.registros.push(rec);
      return rec;
    }
    /* ejecuta una lista de hilos (índices o nombres) en ese orden */
    correr(orden) { orden.forEach((x) => this.paso(typeof x === 'number' ? x : this.hilos.findIndex((h) => h.nombre === x))); return this; }
    cerrar() {   // al final: los hilos sueltan lo que tengan y el cartel se vacía con store(null); todo debe quedar liberado
      if (this.cerrado) return null; this.cerrado = true; this._nuevos = []; this.n++;
      this.hilos.forEach((h) => { while (h.pases.length) { const id = h.pases.pop(); this.tocar(h, id); this.restar(id, 1); } });
      const c = this.cartel;
      this.cartel = { ptr: null, peso: 0 };
      if (c.ptr) this.restar(c.ptr, Math.max(0, c.peso));
      this.orden.forEach((id) => { const L = this.libros[id]; if (L.vivo) this.fallo('fuga', `FUGA: ${id} nunca se liberó (su hoja quedó en ${L.rc}).`); });
      const rec = { n: this.n, hilo: null, hi: -1, acc: 'cierre: se sueltan los pases y store(null)', linea: '', nuevos: this._nuevos.slice(), snap: this.snap() };
      this.registros.push(rec); return rec;
    }
    snap() {
      return clon({
        cartel: this.cartel, P: this.P, C: this.C, variante: this.var, O: this.O,
        libros: this.orden.map((id) => this.libros[id]),
        hilos: this.hilos.map((h) => ({ nombre: h.nombre, pases: h.pases, prox: this.prox(h), hecho: h.hecho, linea: h.linea, pc: h.pc,
          recarga: h.loc && h.loc.recargando && h.pc && h.pc.indexOf('iw.') === 0 ? { p: h.loc.p, w: h.loc.w, d: h.loc.d, cur: h.loc.cur, intentos: h.loc.intentos, reservado: !!h.loc.reservado } : null })),
        respaldo: this.respaldo(), fallos: this.fallos,
      });
    }
  }

  /* ---------- ejecuciones aleatorias y búsqueda de contraejemplos (como el simulador de la Parte 12) ---------- */
  const NOMBRES = ['Ana', 'Beto', 'Carla', 'Dani', 'Eva', 'Fede'];
  function programa(r, nops, pStore) {
    const ops = [];
    for (let k = 0; k < nops; k++) { if (r() < pStore) ops.push('store'); else ops.push('load', 'usar', 'soltar'); }
    return ops;
  }
  /* una ejecución aleatoria completa; se detiene en el primer fallo si o.parar */
  function aleatoria(o, semilla, ligero) {
    const r = rng(semilla);
    const P = o.P; const C = o.C; const v = o.variante || 'paper';
    let peso;
    if (v === 'ingenua') peso = 1;
    else { const lo = v === 'umbral0' ? 1 : P; peso = lo + Math.floor(r() * (C - lo + 1)); }
    const hilos = NOMBRES.slice(0, o.hilos).map((nombre) => ({ nombre, ops: programa(r, o.ops || 3, o.pStore === undefined ? 0.3 : o.pStore) }));
    const s = new Sim({ P, C, variante: v, cartel: { libro: 'L1', peso }, libros: { L1: peso }, hilos, ligero });
    let guard = 0;
    while (!s.terminado() && guard++ < 5000) {
      const l = s.listos(); s.paso(l[Math.floor(r() * l.length)]);
      if (o.parar !== false && s.fallos.length) return s;
    }
    s.cerrar();
    return s;
  }
  function buscar(o, n, semilla0) {
    const res = { ejecuciones: 0, conFallo: 0, tipos: {}, mejor: null, pasos: 0, intentos: {} };
    const semilla = (k) => (semilla0 || 1) * 7919 + k * 104729 + 13;
    let mejorK = -1;
    for (let k = 0; k < n; k++) {
      const s = aleatoria(o, semilla(k), true);
      res.ejecuciones++; res.pasos += s.n;
      s.intentos.forEach((x) => { res.intentos[x] = (res.intentos[x] || 0) + 1; });
      if (s.fallos.length) {
        res.conFallo++; const f = s.fallos[0]; res.tipos[f.tipo] = (res.tipos[f.tipo] || 0) + 1;
        if (!res.mejor || f.paso < res.mejor.paso) { res.mejor = { paso: f.paso, fallo: f }; mejorK = k; }
      }
    }
    if (mejorK >= 0) { const s = aleatoria(o, semilla(mejorK), false); res.mejor.registros = s.registros.slice(0, res.mejor.paso + 1); res.mejor.inicial = s.cfg; }
    return res;
  }

  root.Motor = { Sim, VARIANTES, FALLOS, rng, aleatoria, buscar, NOMBRES };
  if (typeof module !== 'undefined' && module.exports) module.exports = root.Motor;
})(typeof globalThis !== 'undefined' ? globalThis : this);
