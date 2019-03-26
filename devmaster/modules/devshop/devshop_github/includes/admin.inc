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
    $form['connect'] = devshop_github_integration_status_widget();

    $repos = variable_get('devshop_github_available_repositories', array());
    $count = count($repos);
    $form['repos'] = array(
      '#type' => 'container',
      '#prefix' => "<div class='alert alert-info'>",
      '#suffix' => '</div>',
    );

    $form['repos']['message'] = array(
      '#markup' =>  t('@count_string associated with this GitHub token.', array(
        '@count_string' => format_plural($count, t('1 repository found'), t('@count repositories found'))
    )));
    $form['repos']['button'] = array(
      '#markup' => l(t('Refresh repository list'),'admin/devshop/github/load-repos', array('attributes' => array(
      'class' => array('btn btn-link'),
    ))));
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
  else {
    devshop_github_refresh_repositories();
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

/**
 * Grab an authenticated GitHub client.
 *
 * @return \Exception|\Github\Client
 */
function devshop_github_client() {

  $client = new \Github\Client();
  $client->authenticate(variable_get('devshop_github_token'), Github\Client::AUTH_HTTP_TOKEN);

  try {
    $show = $client->currentUser()->show();
    return $client;
  }
  catch (Exception $e) {
    return $e;
  }
}

/**
 * Page callback for "get all repos" link.
 */
function devshop_github_get_repositories_page() {
  $token = variable_get('devshop_github_token', '');
  if (empty($token)) {
    drupal_set_message('GitHub API Token is not set.', 'error');
    drupal_goto('admin/devshop/github');
    return;
  }

  devshop_github_refresh_repositories();

  drupal_goto('admin/devshop/github');
  return;
}

/**
 * Get a list of all repos a user can access.
 */
function devshop_github_refresh_repositories() {

  try {
    $client = devshop_github_client();
    $userApi = $client->currentUser();
    $paginator  = new Github\ResultPager($client);
    $params = array(
      'all'
    );
    $repos = $paginator->fetchAll($userApi, 'repositories', $params);

    foreach ($repos as $repo) {
      $available_repos[$repo['full_name']] = array(
        'url'  => $repo['ssh_url'],
        'org' => $repo['owner']['login'],
      );
    }
    $count = count($repos);
    $r = array(
      '@count' => $count,
    );

    variable_set('devshop_github_available_repositories', $available_repos);

    drupal_set_message(t('Found @count_string associated with that GitHub token.', array(
      '@count_string' => format_plural($count, '1 repository', t('@count repositories', $r)),
    )));
  }
  catch (\Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    drupal_set_message('Code ' . $e->getCode(), 'error');
  }
}