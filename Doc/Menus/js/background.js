// Fond anime (cercles/runes, echo du dessin de sorts) -- purement decoratif, independant du reste.
const canvas = document.getElementById('runes');
const ctx = canvas.getContext('2d');
let w, h;
function resize() { w = canvas.width = window.innerWidth; h = canvas.height = window.innerHeight; }
window.addEventListener('resize', resize);
resize();

const bgShapes = [
  { type: 'circle', x: 0.78, y: 0.35, r: 140, speed: 0.06, color: 'rgba(140,100,255,0.18)' },
  { type: 'square',  x: 0.80, y: 0.38, r: 90,  speed: -0.09, color: 'rgba(232,201,135,0.14)' },
  { type: 'circle', x: 0.80, y: 0.36, r: 190, speed: 0.03, color: 'rgba(80,170,255,0.10)' },
];
function drawSquareBG(cx, cy, r, angle, color) {
  ctx.save(); ctx.translate(cx, cy); ctx.rotate(angle);
  ctx.strokeStyle = color; ctx.lineWidth = 1.5; ctx.strokeRect(-r, -r, r * 2, r * 2); ctx.restore();
}
function drawCircleBG(cx, cy, r, color) {
  ctx.beginPath(); ctx.arc(cx, cy, r, 0, Math.PI * 2); ctx.strokeStyle = color; ctx.lineWidth = 1.5; ctx.stroke();
}
let t = 0;
function loop() {
  ctx.clearRect(0, 0, w, h); t += 0.01;
  bgShapes.forEach(s => {
    const cx = s.x * w, cy = s.y * h, angle = t * s.speed;
    if (s.type === 'circle') drawCircleBG(cx, cy, s.r, s.color); else drawSquareBG(cx, cy, s.r, angle, s.color);
  });
  requestAnimationFrame(loop);
}
loop();
