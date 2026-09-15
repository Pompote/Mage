// Ecran Salon (screen-room) : browser de serveurs factices (JOIN) + panneau de salon
// (JOIN/CREATE), joueurs, pret/lancer. Depend de showScreen (navigation.js) et fadeOut
// (navigation.js), tous deux appeles seulement au clic -- donc l'ordre de chargement des
// <script> n'a pas besoin d'etre strict, mais navigation.js est charge avant par convention.

const browserPanel = document.getElementById('browser-panel');
const roomTitle = document.getElementById('room-title');
const roomSub = document.getElementById('room-sub');
const roomCodeValue = document.getElementById('room-code-value');
const slotsEl = document.getElementById('slots');
const inviteInput = document.getElementById('invite-input');
const btnReady = document.getElementById('btn-ready');
const btnStart = document.getElementById('btn-start');
const ACCENTS = ['gold', 'feu', 'eau', 'terre', 'air'];

// Quelques salons factices pour le flow JOIN (pas de vrai reseau dans cette maquette).
const FAKE_SERVERS = [
  { code: 'ROM1-42', name: 'Salon de Romain', players: [
      { name: 'Romain', host: true, ready: true },
      { name: 'Nova', host: false, ready: true },
    ] },
  { code: 'DNJ9-07', name: 'Donjon express', players: [
      { name: 'Yorick', host: true, ready: false },
    ] },
];

let players = [];

function randomRoomCode() {
  const chars = 'ABCDEFGHJKLMNPQRSTUVWXYZ23456789';
  let s = '';
  for (let i = 0; i < 4; i++) s += chars[Math.floor(Math.random() * chars.length)];
  return 'M4G3-' + s;
}

function renderServerList(filter) {
  const list = document.getElementById('server-list');
  list.innerHTML = '';
  const term = (filter || '').trim().toLowerCase();
  const matches = FAKE_SERVERS.filter(s => !term || s.name.toLowerCase().includes(term));

  if (matches.length === 0) {
    list.innerHTML = '<div class="server-empty-hint">Aucun salon trouvé.</div>';
    return;
  }

  matches.forEach(server => {
    const row = document.createElement('div');
    row.className = 'server-row';
    row.innerHTML = `<span class="srv-name">${server.name}</span><span class="srv-meta">${server.code} — ${server.players.length}/4 joueurs</span>`;
    row.addEventListener('click', () => enterRoom('join', server));
    list.appendChild(row);
  });
}

function enterRoom(mode, server) {
  if (mode === 'create') {
    browserPanel.classList.add('hidden');
    roomTitle.textContent = 'Ton salon';
    roomSub.textContent = '4 joueurs max — invite des amis pour commencer';
    roomCodeValue.textContent = randomRoomCode();
    players = [{ name: 'Toi', isYou: true, host: true, connected: true, pending: false, ready: false }, null, null, null];
  } else {
    browserPanel.classList.remove('hidden');
    roomTitle.textContent = server.name;
    roomSub.textContent = '4 joueurs max — en attente que l\'hôte lance';
    roomCodeValue.textContent = server.code;
    players = server.players.map(p => ({ ...p, connected: true, pending: false }));
    players.push({ name: 'Toi', isYou: true, host: false, connected: true, pending: false, ready: false });
    while (players.length < 4) players.push(null);
  }
  renderSlots();
  resetReadyButton();
  showScreen('screen-room');
}

function renderSlots() {
  slotsEl.innerHTML = '';
  players.forEach((player, index) => {
    const slot = document.createElement('div');
    slot.className = 'slot' + (player ? '' : ' empty');

    if (!player) {
      slot.textContent = '+ Inviter un joueur';
      slot.addEventListener('click', () => inviteInput.focus());
      slotsEl.appendChild(slot);
      return;
    }

    const statusClass = player.pending ? 'is-pending' : (player.ready ? 'is-ready' : '');
    const statusText = player.pending ? 'Connexion...' : (player.ready ? 'Prêt' : 'Pas prêt');
    const tag = player.host ? '<span class="host-tag">Hôte</span>' : (player.isYou ? '<span class="you-tag">Toi</span>' : '');

    slot.innerHTML = `
      <div class="slot-avatar acc-${ACCENTS[index % ACCENTS.length]}">${player.name.charAt(0).toUpperCase()}</div>
      <div class="slot-info">
        <div class="slot-name">${player.name}${tag}</div>
        <div class="slot-status ${statusClass}">${statusText}</div>
      </div>
    `;
    slotsEl.appendChild(slot);
  });
  updateStartButton();
}

function resetReadyButton() {
  const you = players.find(p => p && p.isYou);
  const ready = you ? you.ready : false;
  btnReady.classList.toggle('is-ready', ready);
  btnReady.childNodes[0].textContent = ready ? 'Prêt' : 'Pas prêt';
}

function updateStartButton() {
  const you = players.find(p => p && p.isYou);
  const connected = players.filter(p => p && p.connected);
  // Coop : impossible de lancer seul, et il faut etre l'hote pour voir le bouton actif
  // ("LANCER LA PARTIE (uniquement pour hôte)" dans le schema d'origine).
  const allReady = connected.length > 1 && connected.every(p => p.ready);
  btnStart.disabled = !(you && you.host && allReady);
}

document.getElementById('btn-server-ok').addEventListener('click', () => renderServerList(document.getElementById('server-search').value));
document.getElementById('server-search').addEventListener('input', (e) => renderServerList(e.target.value));

document.getElementById('btn-invite').addEventListener('click', () => {
  const emptyIndex = players.findIndex(p => p === null);
  if (emptyIndex === -1) return;
  const name = inviteInput.value.trim() || 'Ami';
  players[emptyIndex] = { name, host: false, connected: false, pending: true, ready: false };
  inviteInput.value = '';
  renderSlots();
  setTimeout(() => {
    if (!players[emptyIndex]) return;
    players[emptyIndex].pending = false;
    players[emptyIndex].connected = true;
    players[emptyIndex].ready = true;
    renderSlots();
  }, 1100);
});

btnReady.addEventListener('click', () => {
  const you = players.find(p => p && p.isYou);
  if (!you) return;
  you.ready = !you.ready;
  resetReadyButton();
  updateStartButton();
});

btnStart.addEventListener('click', () => {
  if (btnStart.disabled) return;
  fadeOut.querySelector('span').textContent = 'Lancement de la partie...';
  fadeOut.classList.add('active');
  setTimeout(() => { fadeOut.classList.remove('active'); showScreen('screen-main'); }, 1400);
});

document.getElementById('btn-copy-code').addEventListener('click', (e) => {
  const code = roomCodeValue.textContent;
  const btn = e.currentTarget;
  const original = btn.textContent;
  try { navigator.clipboard.writeText(code); } catch (err) { /* presse-papier bloque en file:// -- feedback visuel quand meme */ }
  btn.textContent = 'Copié !';
  setTimeout(() => { btn.textContent = original; }, 1200);
});

document.getElementById('btn-join').addEventListener('click', () => { renderServerList(''); enterRoom('join', FAKE_SERVERS[0]); });
document.getElementById('btn-create').addEventListener('click', () => enterRoom('create'));
