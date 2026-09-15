// Ecran Spell Kit Selector (screen-kit), depuis SOLO. Depend de SPELL_POOL/shapeSVG
// (spell-data.js), fadeOut/showScreen (navigation.js).

const kitCardsEl = document.getElementById('kit-cards');
const kitSlotsEl = document.getElementById('kit-slots');
const btnValidate = document.getElementById('btn-validate');
let kitStart = 0;
const KIT_VISIBLE = 4;
// 3 emplacements, un par touche de cast reelle du jeu (voir README section 3 : "spell cast: a, e, r").
const SLOT_KEYS = ['A', 'E', 'R'];
let loadout = [null, null, null];

function renderKitCarousel() {
  kitCardsEl.innerHTML = '';
  const visible = SPELL_POOL.slice(kitStart, kitStart + KIT_VISIBLE);
  visible.forEach(spell => {
    const inLoadout = loadout.some(s => s && s.id === spell.id);
    const card = document.createElement('div');
    card.className = 'panel kit-card' + (inLoadout ? ' in-loadout' : '');
    card.draggable = true;
    card.innerHTML = `${shapeSVG(spell.shape)}<span>${spell.name}</span>`;
    card.addEventListener('click', () => toggleLoadout(spell));
    card.addEventListener('dragstart', (e) => {
      e.dataTransfer.setData('text/plain', spell.id);
      e.dataTransfer.effectAllowed = 'copy';
      card.classList.add('dragging');
    });
    card.addEventListener('dragend', () => card.classList.remove('dragging'));
    kitCardsEl.appendChild(card);
  });
  document.getElementById('kit-prev').disabled = kitStart === 0;
  document.getElementById('kit-next').disabled = kitStart + KIT_VISIBLE >= SPELL_POOL.length;
}

function renderKitSlots() {
  kitSlotsEl.innerHTML = '';
  loadout.forEach((spell, index) => {
    const slot = document.createElement('div');
    slot.className = 'kit-slot' + (spell ? ' filled' : '');
    slot.innerHTML = `<div class="kit-slot-key">${SLOT_KEYS[index]}</div>` + (spell ? `${shapeSVG(spell.shape)}<span>${spell.name}</span>` : '+');

    slot.addEventListener('click', () => { if (spell) assignToSlot(null, index); });

    // Un slot deja rempli peut aussi etre glisse (pour echanger avec un autre slot).
    slot.draggable = !!spell;
    slot.addEventListener('dragstart', (e) => {
      if (!spell) return;
      e.dataTransfer.setData('text/plain', spell.id);
      e.dataTransfer.effectAllowed = 'move';
    });

    slot.addEventListener('dragover', (e) => { e.preventDefault(); e.dataTransfer.dropEffect = 'copy'; slot.classList.add('drag-over'); });
    slot.addEventListener('dragleave', () => slot.classList.remove('drag-over'));
    slot.addEventListener('drop', (e) => {
      e.preventDefault();
      slot.classList.remove('drag-over');
      const spellId = e.dataTransfer.getData('text/plain');
      assignToSlot(spellId, index);
    });

    kitSlotsEl.appendChild(slot);
  });
  updateValidate();
}

// Assigne (ou retire, si spellId est null) un sort a un emplacement precis -- utilise a la fois
// par le drag-and-drop (depuis le carrousel ou depuis un autre slot) et par le clic simple.
function assignToSlot(spellId, targetIndex) {
  if (!spellId) { loadout[targetIndex] = null; renderKitCarousel(); renderKitSlots(); return; }
  const spell = SPELL_POOL.find(s => s.id === spellId);
  if (!spell) return;
  const existingIndex = loadout.findIndex(s => s && s.id === spell.id);
  if (existingIndex !== -1) loadout[existingIndex] = null;
  loadout[targetIndex] = spell;
  renderKitCarousel();
  renderKitSlots();
}

function toggleLoadout(spell) {
  const existingIndex = loadout.findIndex(s => s && s.id === spell.id);
  if (existingIndex !== -1) { assignToSlot(null, existingIndex); return; }
  const emptyIndex = loadout.findIndex(s => s === null);
  if (emptyIndex === -1) return; // A, E et R deja remplis
  assignToSlot(spell.id, emptyIndex);
}

function updateValidate() { btnValidate.disabled = loadout.every(s => !s); }

document.getElementById('kit-prev').addEventListener('click', () => { kitStart = Math.max(0, kitStart - 1); renderKitCarousel(); });
document.getElementById('kit-next').addEventListener('click', () => { kitStart = Math.min(SPELL_POOL.length - KIT_VISIBLE, kitStart + 1); renderKitCarousel(); });

document.getElementById('btn-solo').addEventListener('click', () => {
  kitStart = 0; loadout = [null, null, null];
  renderKitCarousel(); renderKitSlots();
  showScreen('screen-kit');
});

btnValidate.addEventListener('click', () => {
  if (btnValidate.disabled) return;
  fadeOut.querySelector('span').textContent = 'Chargement...';
  fadeOut.classList.add('active');
  setTimeout(() => { fadeOut.classList.remove('active'); showScreen('screen-main'); }, 1400);
});
