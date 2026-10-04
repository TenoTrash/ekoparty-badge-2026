const printButtons = document.querySelectorAll('[data-print]');
for (const button of printButtons) {
  button.addEventListener('click', () => window.print());
}

const menuToggle = document.querySelector('.menu-toggle');
const menu = document.querySelector('#guide-menu');
const menuBackdrop = document.querySelector('.menu-backdrop');
const mobileMenu = window.matchMedia('(max-width: 900px)');

function setMenuOpen(open) {
  const isOpen = open && mobileMenu.matches;
  document.body.classList.toggle('menu-open', isOpen);
  menuToggle.setAttribute('aria-expanded', String(isOpen));
  menuToggle.setAttribute('aria-label', isOpen ? 'Cerrar menú de capítulos' : 'Abrir menú de capítulos');
  menuBackdrop.hidden = !isOpen;
  menu.inert = mobileMenu.matches && !isOpen;
}

menuToggle.addEventListener('click', () => setMenuOpen(!document.body.classList.contains('menu-open')));
menuBackdrop.addEventListener('click', () => {
  setMenuOpen(false);
  menuToggle.focus();
});
menu.querySelectorAll('a[href^="#"]').forEach((link) => {
  link.addEventListener('click', () => setMenuOpen(false));
});
document.addEventListener('keydown', (event) => {
  if (event.key === 'Escape' && document.body.classList.contains('menu-open')) {
    setMenuOpen(false);
    menuToggle.focus();
  }
});
mobileMenu.addEventListener('change', () => setMenuOpen(false));
setMenuOpen(false);

const printDetails = [...document.querySelectorAll('details')];
let openDetailsBeforePrint = [];
window.addEventListener('beforeprint', () => {
  openDetailsBeforePrint = printDetails.filter((detail) => detail.open);
  for (const detail of printDetails) detail.open = true;
});
window.addEventListener('afterprint', () => {
  for (const detail of printDetails) detail.open = openDetailsBeforePrint.includes(detail);
});

const lightbox = document.querySelector('.lightbox');
const lightboxImage = lightbox.querySelector('img');
const lightboxCaption = lightbox.querySelector('p');
const closeButton = lightbox.querySelector('.lightbox-close');

if (typeof lightbox.showModal === 'function') {
  for (const link of document.querySelectorAll('[data-lightbox]')) {
    link.addEventListener('click', (event) => {
      event.preventDefault();
      const source = link.querySelector('img');
      lightboxImage.src = link.href;
      lightboxImage.alt = source.alt;
      lightboxCaption.textContent = link.closest('figure')?.querySelector('figcaption')?.textContent || '';
      lightbox.showModal();
      closeButton.focus();
    });
  }

  closeButton.addEventListener('click', () => lightbox.close());
  lightbox.addEventListener('click', (event) => {
    if (event.target === lightbox) lightbox.close();
  });
  lightbox.addEventListener('close', () => {
    lightboxImage.removeAttribute('src');
  });
}
