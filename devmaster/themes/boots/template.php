<?php

/**
 * Returns HTML for primary and secondary local tasks.
 *
 * @param array $variables
 *   An associative array containing:
 *     - primary: (optional) An array of local tasks (tabs).
 *     - secondary: (optional) An array of local tasks (tabs).
 *
 * @return string
 *   The constructed HTML.
 *
 * @see theme_menu_local_tasks()
 * @see menu_local_tasks()
 *
 * @ingroup theme_functions
 */
function boots_menu_local_tasks(&$variables) {
  $output = '';

  if (!empty($variables['primary'])) {
    $variables['primary']['#prefix'] = '<h2 class="element-invisible">' . t('Primary tabs') . '</h2>';
    $variables['primary']['#prefix'] .= '<ul class="tabs--primary nav nav-pills nav-stacked">';
    $variables['primary']['#suffix'] = '</ul>';
    $output .= drupal_render($variables['primary']);
  }

  if (!empty($variables['secondary'])) {
    $variables['secondary']['#prefix'] = '<h2 class="element-invisible">' . t('Secondary tabs') . '</h2>';
    $variables['secondary']['#prefix'] .= '<ul class="tabs--secondary pagination pagination-sm">';
    $variables['secondary']['#suffix'] = '</ul>';
    $output .= drupal_render($variables['secondary']);
  }

  return $output;
}

/**
 * Implements hook_form_FORM_ID_alter() for node_site_form
 *
 * "Environment" Settings form.
 */
function boots_form_project_node_form_alter(&$form, &$form_state, $form_id) {

  $form['#prefix'] = '<h3>' . t('Project Settings') . '</h3>';

}
