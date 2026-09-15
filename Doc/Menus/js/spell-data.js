// Donnees et pictogrammes partages par l'ecran Sorts (grimoire) et le Spell Kit Selector.
// Aucune dependance sur les autres fichiers JS -- peut etre charge en premier ou en dernier,
// tant que c'est avant spells.js/kit.js qui appellent ces fonctions.

function shapeSVG(type, colorVar) {
  const stroke = colorVar ? `var(${colorVar})` : 'currentColor';
  if (type === 'circle') return `<svg viewBox="0 0 24 24" fill="none"><circle cx="12" cy="12" r="9" stroke="${stroke}" stroke-width="1.4"/></svg>`;
  if (type === 'square') return `<svg viewBox="0 0 24 24" fill="none"><rect x="4" y="4" width="16" height="16" stroke="${stroke}" stroke-width="1.4"/></svg>`;
  if (type === 'zigzag') return `<svg viewBox="0 0 24 24" fill="none"><polyline points="2,19 7,5 12,19 17,5 22,19" stroke="${stroke}" stroke-width="1.4" stroke-linejoin="round" stroke-linecap="round"/></svg>`;
  if (type === 'spiral') {
    const pts = [];
    const turns = 2.5, steps = 60, maxR = 10;
    for (let i = 0; i <= steps; i++) {
      const frac = i / steps, angle = frac * turns * Math.PI * 2, r = frac * maxR;
      pts.push(`${(12 + r * Math.cos(angle)).toFixed(2)},${(12 + r * Math.sin(angle)).toFixed(2)}`);
    }
    return `<svg viewBox="0 0 24 24" fill="none"><polyline points="${pts.join(' ')}" stroke="${stroke}" stroke-width="1.2" fill="none"/></svg>`;
  }
  return '';
}

// ================= Elements + reactions (README, "Mecanique #2 : Elements et interactions") =================
const ELEMENT_LABEL = { feu: 'Feu', eau: 'Eau', terre: 'Terre', air: 'Air' };

function elementSVG(key) {
  const c = key ? `var(--${key})` : 'var(--text-dim)';
  if (key === 'feu') return `<svg viewBox="0 0 24 24"><path fill="${c}" d="M12 21c-4 0-7-2.7-7-6.5C5 10 8 7 9 4c.3 2 1.5 3 2.5 2-.4 2 1.3 3 1.3 5.6.9-.9 1.4-2.3 1.4-3.6C17 9.5 19 12 19 14.5 19 18.3 16 21 12 21z"/></svg>`;
  if (key === 'eau') return `<svg viewBox="0 0 24 24"><path fill="${c}" d="M12 3C9 8 5 12 5 16a7 7 0 0014 0c0-4-4-8-7-13z"/></svg>`;
  if (key === 'terre') return `<svg viewBox="0 0 24 24"><path fill="${c}" d="M3 18l4-9 3 3 3-7 4 8 4 5H3z"/></svg>`;
  if (key === 'air') return `<svg viewBox="0 0 24 24" fill="none"><path stroke="${c}" stroke-width="1.6" stroke-linecap="round" d="M3 8h11a3 3 0 10-3-3"/><path stroke="${c}" stroke-width="1.6" stroke-linecap="round" d="M3 13h15a3 3 0 11-3 3"/><path stroke="${c}" stroke-width="1.6" stroke-linecap="round" d="M3 18h9a3 3 0 10-3-3"/></svg>`;
  // Pas d'element assigne -- champ optionnel (Doc/WORKFLOW.md, workflow 2)
  return `<svg viewBox="0 0 24 24" fill="none"><circle cx="12" cy="12" r="8" stroke="${c}" stroke-width="1.3" stroke-dasharray="3 3"/></svg>`;
}

// Table des 6 reactions (README, Mecanique #2, "Table des combinaisons").
const REACTIONS = [
  { pair: ['feu', 'eau'], id: 'brume', name: 'Brume' },
  { pair: ['eau', 'terre'], id: 'boue', name: 'Boue' },
  { pair: ['feu', 'terre'], id: 'roche', name: 'Roche incandescente' },
  { pair: ['feu', 'air'], id: 'foudre', name: 'Foudre' },
  { pair: ['eau', 'air'], id: 'trombe', name: "Trombe d'eau" },
  { pair: ['terre', 'air'], id: 'poussiere', name: 'Tempête de poussière' },
];
function reactionsFor(element) {
  if (!element) return [];
  return REACTIONS.filter(r => r.pair.includes(element)).map(r => ({ other: r.pair.find(e => e !== element), id: r.id, name: r.name }));
}

function reactionSVG(id) {
  switch (id) {
    case 'brume': return `<svg viewBox="0 0 24 24" fill="none"><path stroke="var(--air)" stroke-width="1.4" d="M3 9c2-2 4-2 6 0s4 2 6 0 4-2 6 0"/><path stroke="var(--air)" stroke-width="1.4" d="M3 14c2-2 4-2 6 0s4 2 6 0 4-2 6 0"/><path stroke="var(--air)" stroke-width="1.4" d="M3 19c2-2 4-2 6 0s4 2 6 0 4-2 6 0"/></svg>`;
    case 'boue': return `<svg viewBox="0 0 24 24"><ellipse cx="12" cy="17" rx="8" ry="3.4" fill="#8a7346"/><circle cx="8" cy="9" r="2" fill="#8a7346"/><circle cx="15" cy="7" r="1.3" fill="#8a7346"/></svg>`;
    case 'roche': return `<svg viewBox="0 0 24 24"><path fill="#5a4030" d="M3 18l4-8 3 2.5 3-6 4 7 4 4.5H3z"/><path fill="var(--feu)" d="M11 17l1.5-3 1 2 1.5-2.5 1.5 3.5z"/></svg>`;
    case 'foudre': return `<svg viewBox="0 0 24 24"><path fill="#fce98a" d="M13 2L4 14h6l-1 8 10-13h-6z"/></svg>`;
    case 'trombe': return `<svg viewBox="0 0 24 24" fill="none"><path stroke="var(--eau)" stroke-width="1.5" stroke-linecap="round" d="M4 5h16M6.5 9.5h11M9 14h6M11 18.5h2"/></svg>`;
    case 'poussiere': return `<svg viewBox="0 0 24 24" fill="none"><circle cx="6" cy="10" r="1.4" fill="#c9b27a"/><circle cx="11" cy="7" r="1" fill="#c9b27a"/><circle cx="16" cy="11" r="1.6" fill="#c9b27a"/><circle cx="9" cy="15" r="1.2" fill="#c9b27a"/><circle cx="15" cy="16" r="1" fill="#c9b27a"/><path stroke="#c9b27a" stroke-width="1.2" fill="none" d="M3 19c3-2 15-2 18 0"/></svg>`;
    default: return '';
  }
}

// Pool de sorts : Circle/Square sont les 2 sorts reellement implementes dans le projet
// (SD_Circle/SD_Square) ; Spiral/Z reprennent les exemples de formes cites dans le README
// (mecanique #1). Le reste (Eclair, Tornade, Vague...) est ajoute uniquement pour avoir assez
// d'entrees et tester la pagination du carrousel/la liste -- pas de vrais sorts concus.
// ATTENTION "element" : placeholders de maquette, pas les vraies valeurs de SD_Circle/SD_Square
// -- la connexion MCP a l'editeur Unreal etait indisponible cette session pour les lire
// reellement (voir DataAssetTools). A corriger une fois confirme. "rune" n'a volontairement
// aucun element, pour illustrer que le champ Element est optionnel (Doc/WORKFLOW.md, workflow 2).
const SPELL_POOL = [
  { id: 'circle', name: 'Circle', shape: 'circle', element: 'feu', desc: 'Sort de base : projette un projectile qui explose au contact (voir BP_CircleSpellEffect).' },
  { id: 'square', name: 'Square', shape: 'square', element: 'terre', desc: 'Sort de base : invoque un projectile de roche (voir workflow "Square casts a rock projectile").' },
  { id: 'spiral', name: 'Spiral', shape: 'spiral', element: 'eau', desc: '(Prototype) Forme citee dans le README comme exemple -- effet pas encore defini.' },
  { id: 'zigzag', name: 'Z', shape: 'zigzag', element: 'air', desc: '(Prototype) Forme citee dans le README comme exemple -- effet pas encore defini.' },
  { id: 'eclair', name: 'Éclair', shape: 'zigzag', element: 'air', desc: '(Exemple test) Ajoute pour peupler la navigation -- pas un sort conçu.' },
  { id: 'tornade', name: 'Tornade', shape: 'spiral', element: 'air', desc: '(Exemple test) Ajoute pour peupler la navigation -- pas un sort conçu.' },
  { id: 'vague', name: 'Vague', shape: 'circle', element: 'eau', desc: '(Exemple test) Ajoute pour peupler la navigation -- pas un sort conçu.' },
  { id: 'fracture', name: 'Fracture', shape: 'square', element: 'terre', desc: '(Exemple test) Ajoute pour peupler la navigation -- pas un sort conçu.' },
  { id: 'comete', name: 'Comète', shape: 'spiral', element: 'feu', desc: '(Exemple test) Ajoute pour peupler la navigation -- pas un sort conçu.' },
  { id: 'rune', name: 'Rune', shape: 'square', element: null, desc: '(Exemple test) Ajoute pour peupler la navigation -- pas un sort conçu.' },
];
