<?php

/**
 * GitHub API key form.
 * @return array()
 */
function devshop_github_settings_form(){
  $form['devshop_github_token'] = array(
    '#title' => t('GitHub API Token'),
    '#type' => 'password',
    '#description' => t('A GitHub API Token or Personal Access Token'),
    '#default_value' => variable_get('devshop_github_token', ''),
  );
  $github_url = "https://github.com/settings/tokens";
  $form['help'] = array(
      '#value' => t('Manage your tokens under account settings at !link.', array(
        '!link' => l($github_url, $github_url, array('attributes' => array('target' => '_blank'))),
      )) . '</p><p>' . t('Be sure to select the following Oauth Scopes: <em>repo, repo:status, repo:deployment, public_repo, write:public_key, write_repo_hook</em>'),
      '#prefix' => '<div class="well well-sm"><p>',
      '#suffix' => '</p></div>',
  );


  // Warn the user if this is empty.
  $token = variable_get('devshop_github_token', '');
  if (empty($token)) {
    $form['devshop_github_token']['#description'] .= ' <em class="warning">' . t('No Token was found.') . '</em>';
    $form['devshop_github_token']['#description'] .= '  ' . l(t('Create a new token'), 'https://github.com/settings/tokens/new', array('attributes' => array('target' => '_blank')));
  }
  else {
      $form['devshop_github_token']['#description'] .= ' <em class="text-success">' . t('Your token is saved.') . '</em>';
  }

  return system_settings_form($form);
}
