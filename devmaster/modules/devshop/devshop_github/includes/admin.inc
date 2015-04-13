<?php

/**
 * GitHub API key form.
 * @return array()
 */
function devshop_github_settings_form(){
  $form['devshop_github_token'] = array(
    '#title' => t('GitHub API Token'),
    '#type' => 'password',
    '#description' => t('A GitHub API Token'),
    '#default_value' => variable_get('devshop_github_token', ''),
  );
  return system_settings_form($form);
}