const printButtons = document.querySelectorAll('[data-print]');
for (const button of printButtons) {
  button.addEventListener('click', () => window.print());
}

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
