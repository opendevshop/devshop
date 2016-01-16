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
  
  // Test for public key access.
  if (!empty($token)) {
    try {

      $client = new \Github\Client();
      $client->authenticate($token, Github\Client::AUTH_HTTP_TOKEN);
      $keys = $client->currentUser()->keys()->all();
    } 
    // Happens when user has no public keys.
    catch (\GitHub\Exception\RuntimeException $e) {
      drupal_set_message(t('Something went wrong trying to request http://api.github.com/user/keys.  Make sure your personal token is authorized for repo, repo:status, repo:deployment, public_repo, write:public_key, & write:repo_hook.'), 'error');
      return system_settings_form($form);
    }

    $form['github_public_keys'] = array(
        '#value' => '',
    );
    
    $devshop_key = variable_get('devshop_public_key', '');

    $found_key = FALSE;
    foreach ($keys as $key) {
        if (strpos($devshop_key, $key['key']) !== FALSE){
            $found_key = TRUE;
        }
    }

    if ($found_key) {
        $form['github_public_keys']['#value'] = ' <div class="alert alert-success">' . t('The DevShop Public SSH Key has been found in your GitHub account.') . '</div>';
    }
    else {
        $form['github_public_keys']['#value'] = t('The DevShop Public SSH Key was not found in your account.') . '  ';

        $form['github_public_keys']['#value'] .= l(t('Add devshop public key to your GitHub account'), 'admin/devshop/github/add-key', array(
                'attributes' => array(
                  'class' => 'btn btn-success'
                ),
            )) . '</div>';
        $form['github_public_keys']['#prefix'] = '<div class="alert alert-danger">';
        $form['github_public_keys']['#suffix'] = '</div>';
    }
  }

  return system_settings_form($form);
}
