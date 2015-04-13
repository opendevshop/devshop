<?php

/**
 * GitHub API key form.
 * @return array()
 */
function devshop_github_settings_form(){
  $form['devshop_github_api_key'] = array(
    '#title' => t('GitHub API Key'),
    '#type' => 'textfield',
    '#description' => t('A GitHub API key'),
    '#default_value' => variable_get('devshop_github_api_key', ''),
  );
  return system_settings_form($form);
}