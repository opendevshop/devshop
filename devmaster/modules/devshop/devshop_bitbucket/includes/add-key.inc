<?php

/**
 * Page callback for "add key to bitbucket" link.
 */
function devshop_bitbucket_add_key_to_account($form, &$form_state) {

    // Check for token
    $token = variable_get('devshop_bitbucket_token', '');
    $devshop_key = variable_get('devshop_public_key', '');

    if (empty($token)) {
        drupal_set_message('bitbucket API Token is not set.', 'error');
        drupal_goto('admin/hosting');
        return;
    }

    // If key was already found, skip.
    if (devshop_bitbucket_check_key()) {
        drupal_set_message('SSH Key was found already on your account.', 'error');
        drupal_goto('admin/hosting');
        return;
    }

    // Save key to account
    $key_was_added = FALSE;
    $params = array(
        'key' => $devshop_key,
        'title' => t('Added by DevShop from !url', array(
            '!url' => $_SERVER['HTTP_HOST'],
        )),
    );

    // Create public key
    $client = new \bitbucket\Client();
    $client->authenticate($token, bitbucket\Client::AUTH_HTTP_TOKEN);
    try {
      $return = $client->currentUser()->keys()->create($params);
    }
    catch (\bitbucket\Exception\ValidationFailedException $e) {
        drupal_set_message($e->getMessage(), 'error');
        drupal_set_message('Code ' . $e->getCode(), 'error');
    }

    if ($return['verified']) {
        drupal_set_message('SSH Key added to your account.');
        drupal_goto('admin/hosting');
    } else {
        drupal_set_message('Something went wrong. SSH Key was not added to your account.', 'error');
        drupal_goto('admin/hosting');
    }
}
