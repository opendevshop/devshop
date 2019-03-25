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
    '#element_validate' => array(
      'devshop_github_settings_form_validate_token',
    ),
  );

  $form['actions']['devshop_github_token_button'] = array(
    '#weight' => 10,
    '#markup' => l(t('Create a new token on GitHub.com'), 'https://github.com/settings/tokens/new?scopes=repo,admin:public_key,admin:repo_hook&description=' . $_SERVER['HTTP_HOST'], array('attributes' => array(
      'target' => '_blank',
      'class' => array('btn btn-primary  form-submit'),
    )))
  );

  // Warn the user if this is empty.
  $token = variable_get('devshop_github_token', '');
  if (empty($token)) {
    $form['devshop_github_token']['#description'] = ' <em class="text-danger">' . t('No Token was found. Click the button below to create a new token on GitHuub.com') . '</em>';
  }
  else {
    $form['devshop_github_token']['#description'] = ' <em class="text-success">' . t('Your token is saved. Enter a new one if you wish to change it.') . '</em>';
  }

  // Test for public key access.
  if (!empty($token)) {

    $form['github_public_keys'] = array(
        '#markup' => '',
    );

    if (devshop_github_check_key()) {
        $form['github_public_keys']['#markup'] = ' <div class="alert alert-success">' . t("!key_link has been found in your GitHub account.", array(
          '!key_link' => l(t("This DevShop's Public SSH Key"), 'admin/devshop')
          )) . '</div>';
    }
    elseif (!devshop_public_key_valid_fingerprint()) {
      $form['github_public_keys']['#markup'] = ' <div class="alert alert-danger">' . t("Invalid SSH key") . '</div>';
    }
    else {
      $form['github_add_key_message'] = array(
        '#prefix' => '<div class="alert alert-warning">',
        '#markup' => t("!key_link was not found in your account.", array(
          '!key_link' => l(t("This DevShop's Public SSH Key"), 'admin/devshop')
        )),
      );
      $form['github_add_key_button'] = array(
        '#markup' =>  l('<i class="fa fa-github"></i> ' . t('Add devshop public key to your GitHub account'), 'admin/devshop/github/add-key', array(
          'attributes' => array(
            'class' => 'btn btn-default btn-large text-muted'
          ),
          'html' => TRUE,
        )),
        '#suffix' => '</div>',
      );
    }
  }

  $form = system_settings_form($form);
  $form['actions']['submit']['#value'] = t('Save Token');
  return $form;
}

/**
 * Element validation for License Key. Pings devshop.support
 *
 * @param $element
 * @param $form_state
 * @param $form
 */
function devshop_github_settings_form_validate_token($element, &$form_state, $form) {

  $token = $form_state['values']['devshop_github_token'];
  $e = devshop_github_token_is_valid($token);
  if (is_subclass_of($e, 'Exception')) {
    form_error($element,  $e->getMessage());
  }
}

function devshop_github_token_is_valid($token = NULL) {

  if (empty($token)) {
    $token = variable_get('devshop_github_token');
  }

  $client = new \Github\Client();
  $client->authenticate($token, Github\Client::AUTH_HTTP_TOKEN);

  try {
    $show = $client->currentUser()->show();
    return TRUE;
  }
    // Happens when user has no public keys.
  catch (Exception $e) {
    return $e;
  }
}