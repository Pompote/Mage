// Navigation generique entre ecrans + chrome partage (flyout multijoueur, overlays Parametres/
// Quitter, sliders). Charge apres background.js et avant les scripts specifiques a un ecran
// (room.js/spells.js/kit.js), qui appellent showScreen(...) mais ne le definissent pas.

const screens = document.querySelectorAll('.screen');
const multiplayerFlyout = document.getElementById('multiplayer-flyout');

function showScreen(id) {
  screens.forEach(s => s.classList.toggle('active', s.id === id));
  multiplayerFlyout.classList.remove('open');
}

// N'importe quel ecran qui a besoin d'un retour ajoute juste class="btn return-btn" dans son
// HTML -- pas de cablage JS a ecrire ecran par ecran (source du bug precedent : un retour
// oublie = un ecran sans sortie).
document.querySelectorAll('.return-btn').forEach(btn => btn.addEventListener('click', () => showScreen('screen-main')));

// Flyout JOIN/CREATE ancre au bouton MULTIPLAYER du menu principal.
document.getElementById('btn-multiplayer').addEventListener('click', (e) => {
  e.stopPropagation();
  multiplayerFlyout.classList.toggle('open');
});
document.addEventListener('click', (e) => {
  if (!e.target.closest('.menu-btn-wrap')) multiplayerFlyout.classList.remove('open');
});

// ================= Overlays generiques (Settings / Quitter / fondu) =================
const overlayOptions = document.getElementById('overlay-options');
const overlayQuit = document.getElementById('overlay-quit');
const fadeOut = document.getElementById('fade-out');

document.getElementById('btn-settings').addEventListener('click', () => overlayOptions.classList.add('open'));
document.getElementById('btn-close-options').addEventListener('click', () => overlayOptions.classList.remove('open'));

document.getElementById('btn-quit').addEventListener('click', () => overlayQuit.classList.add('open'));
document.getElementById('btn-cancel-quit').addEventListener('click', () => overlayQuit.classList.remove('open'));
document.getElementById('btn-confirm-quit').addEventListener('click', () => {
  overlayQuit.classList.remove('open');
  fadeOut.querySelector('span').textContent = 'Fermeture...';
  fadeOut.classList.add('active');
});

['master', 'music', 'sens'].forEach(id => {
  const range = document.getElementById('range-' + id);
  const val = document.getElementById('val-' + id);
  range.addEventListener('input', () => { val.textContent = range.value + '%'; });
});
