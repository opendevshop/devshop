<?php

/**
 * bitbucket API key form.
 * @return array()
 */
function devshop_bitbucket_settings_form($form, &$form_state) {
  $form['devshop_bitbucket_token'] = array(
    '#title' => t('bitbucket API Token'),
    '#type' => 'password',
    '#description' => t('A bitbucket API Token or Personal Access Token'),
    '#default_value' => variable_get('devshop_bitbucket_token', ''),
  );
  $bitbucket_url = "https://bitbucket.com/settings/tokens";
  $form['help'] = array(
      '#markup' => t('Manage your tokens under account settings at !link.', array(
        '!link' => l($bitbucket_url, $bitbucket_url, array('attributes' => array('target' => '_blank'))),
      )) . '</p><p>' . t('Be sure to select the following Oauth Scopes: <em>repo, repo:status, repo:deployment, public_repo, write:public_key, write_repo_hook</em>'),
      '#prefix' => '<div class="well well-sm"><p>',
      '#suffix' => '</p></div>',
  );


  // Warn the user if this is empty.
  $token = variable_get('devshop_bitbucket_token', '');
  if (empty($token)) {
    $form['devshop_bitbucket_token']['#description'] .= ' <em class="warning">' . t('No Token was found.') . '</em>';
    $form['devshop_bitbucket_token']['#description'] .= '  ' . l(t('Create a new token'), 'https://bitbucket.com/settings/tokens/new', array('attributes' => array('target' => '_blank')));
  }
  else {
    $form['devshop_bitbucket_token']['#suffix'] .= ' <em class="text-success">' . t('Your token is saved.') . '</em>';
  }

  // Test for public key access.
  if (!empty($token)) {

    $form['bitbucket_public_keys'] = array(
        '#markup' => '',
    );

    if (devshop_bitbucket_check_key()) {
        $form['bitbucket_public_keys']['#markup'] = ' <div class="alert alert-success">' . t('The DevShop Public SSH Key has been found in your bitbucket account.') . '</div>';
    }
    else {
        $form['bitbucket_public_keys']['#markup'] = t('The DevShop Public SSH Key was not found in your account.') . '  ';

        $form['bitbucket_public_keys']['#markup'] .= '</div>';
        $form['bitbucket_public_keys']['#prefix'] = '<div class="alert alert-danger">';
        $form['bitbucket_public_keys']['#suffix'] = l('<i class="fa fa-bitbucket"></i> ' . t('Add devshop public key to your bitbucket account'), 'admin/devshop/bitbucket/add-key', array(
            'attributes' => array(
              'class' => array('btn btn-primary btn-large'),
            ),
            'html' => TRUE,
          )) . '</div>';
    }
  }

  return system_settings_form($form);
}
