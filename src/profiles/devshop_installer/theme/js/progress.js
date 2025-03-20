(function (Drupal) {

  /**
   * Renders a client-side progress bar.
   *
   * This is for the Drupal CMS installer and is not meant to be reused.
   */
  Drupal.theme.progressBar = function (id) {
    const escapedId = Drupal.checkPlain(id);
    return (`
      <p class="cms-installer__subhead">This will only take a moment.</p>
      <div id="${escapedId}" class="progress" aria-live="polite">
      <div class="progress__label">&nbsp;</div>
      <div class="progress__track"><div class="progress__bar"></div></div>
      <div class="progress__percentage visually-hidden"></div>
      <div class="progress__description visually-hidden">&nbsp;</div>
      </div>'
    `);
  };

})(Drupal);
