<?php

/**
 * GitHub API key form.
 * @return array()
 */
function devshop_github_settings_form($form, &$form_state) {
  $form['devshop_github_token'] = array(
    '#title' => t('GitHub API Token'),
    '#type' => 'password',
    '#description' => t('A GitHub API Token or Personal Access Token'),
    '#default_value' => variable_get('devshop_github_token', ''),
  );
  $github_url = "https://github.com/settings/tokens";
  $form['help'] = array(
      '#markup' => t('Manage your tokens under account settings at !link.', array(
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
    $form['devshop_github_token']['#suffix'] .= ' <em class="text-success">' . t('Your token is saved.') . '</em>';
  }

  // Test for public key access.
  if (!empty($token)) {

    $form['github_public_keys'] = array(
        '#markup' => '',
    );

    if (devshop_github_check_key()) {
        $form['github_public_keys']['#markup'] = ' <div class="alert alert-success">' . t('The DevShop Public SSH Key has been found in your GitHub account.') . '</div>';
    }
    else {
        $form['github_public_keys']['#markup'] = t('The DevShop Public SSH Key was not found in your account.') . '  ';

        $form['github_public_keys']['#markup'] .= '</div>';
        $form['github_public_keys']['#prefix'] = '<div class="alert alert-danger">';
        $form['github_public_keys']['#suffix'] = l('<i class="fa fa-github"></i> ' . t('Add devshop public key to your GitHub account'), 'admin/devshop/github/add-key', array(
            'attributes' => array(
              'class' => 'btn btn-primary btn-large'
            ),
            'html' => TRUE,
          )) . '</div>';
    }
  }

  return system_settings_form($form);
}
