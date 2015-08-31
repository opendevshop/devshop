<?php

/**
 * Page callback for "add key to github" link.
 */
function devshop_github_add_key_to_account() {

    // Check for token
    $token = variable_get('devshop_github_token', '');
    if (empty($token)) {
        drupal_set_message('GitHub API Token is not set.', 'error');
        drupal_goto('admin/devshop/github');
        return;
    }

    print "Token found!";

    // Check for SSH key
    $client = new \Github\Client();
    $client->authenticate($token, Github\Client::AUTH_HTTP_TOKEN);

    $keys = $client->currentUser()->keys()->all();
    $devshop_key = variable_get('devshop_public_key', '');

    $found_key = FALSE;
    foreach ($keys as $key) {
        if (strpos($devshop_key, $key['key']) !== FALSE){
            $found_key = TRUE;
        }
    }

    // If key was already found, skip.
    if ($found_key == TRUE) {
        drupal_set_message('SSH Key was found already on your account.', 'error');
        drupal_goto('admin/devshop/github');
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

    $return = $client->currentUser()->keys()->create($params);
    if ($return['verified']) {
        drupal_set_message('SSH Key added to your account.');
        drupal_goto('admin/devshop/github');
    }
    else {
        drupal_set_message('Something went wrong. SSH Key was not added to your account.', 'error');
        drupal_goto('admin/devshop/github');
    }
    return;
}