// Ecran Sorts / grimoire (screen-spells). Depend de SPELL_POOL/shapeSVG/elementSVG/reactionsFor/
// reactionSVG/ELEMENT_LABEL (spell-data.js) et showScreen (navigation.js).

const spellListEl = document.getElementById('spell-list');
const spellDescTitle = document.getElementById('spell-desc-title');
const spellDescBody = document.getElementById('spell-desc-body');
let selectedSpellId = null;

function renderSpellList() {
  spellListEl.innerHTML = '';
  SPELL_POOL.forEach(spell => {
    const row = document.createElement('div');
    row.className = 'spell-row' + (spell.id === selectedSpellId ? ' selected' : '');
    row.innerHTML = `<span class="glyph">${shapeSVG(spell.shape)}</span><span class="mini-icon">${elementSVG(spell.element)}</span><span>${spell.name}</span>`;
    row.addEventListener('click', () => selectSpell(spell.id));
    spellListEl.appendChild(row);
  });
}

function selectSpell(id) {
  selectedSpellId = id;
  const spell = SPELL_POOL.find(s => s.id === id);
  spellDescTitle.textContent = spell.name;

  const reactions = reactionsFor(spell.element);
  const reactionsHTML = spell.element
    ? `<div class="reactions-label">Réactions</div>` + reactions.map(r => `
        <div class="reaction-row">
          <span class="mini-icon">${elementSVG(r.other)}</span>
          <span class="reaction-arrow">→</span>
          <span class="mini-icon">${reactionSVG(r.id)}</span>
          <span class="reaction-name">${r.name}</span>
        </div>
      `).join('')
    : `<div class="reactions-label">Aucune réaction (sort sans élément)</div>`;

  spellDescBody.innerHTML = `
    <div class="spell-element-row"><span class="mini-icon">${elementSVG(spell.element)}</span>${spell.element ? ELEMENT_LABEL[spell.element] : 'Aucun élément'}</div>
    <p>${spell.desc}</p>
    ${reactionsHTML}
  `;

  // Le <model-viewer> du panneau de droite est un placeholder generique partage (voir
  // assets/generate-placeholder.py) -- pas encore de modele reel par sort a brancher ici.
  renderSpellList();
}

document.getElementById('btn-spells').addEventListener('click', () => {
  showScreen('screen-spells');
  if (!selectedSpellId) selectSpell(SPELL_POOL[0].id);
  else renderSpellList();
});
