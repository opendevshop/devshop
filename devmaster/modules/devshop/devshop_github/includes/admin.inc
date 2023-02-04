<?php

/**
 * GitHub API key form.
 * @return array()
 */
function devshop_github_form_devshop_projects_settings_form_alter(&$form, &$form_state, $form_id) {

  $token = devshop_github_token();
  $token_valid = $token? devshop_github_token_is_valid($token): FALSE;

  $ssh_key = variable_get('devshop_public_key', '');

  $form['github'] = array(
    '#title' => t('GitHub Integration'),
    '#type' => 'fieldset',
    '#group' => 'settings',
    '#weight' => -3,
    '#description' => t('DevShop integrates with GitHub to provide a number of features:'),
  );
  $form['github']['features'] = array(
    '#theme' => 'item_list',
    '#items' => array(
      t('Submit Public Key to GitHub account to grant this server access to clone code.'),
      t('Loads a list of available repositories for creating new projects.'),
      t('Create new repositories on GitHub when adding projects.'),
      t('Post task statuses to GitHub to integrate with Deployments and Commit Status API.'),
      t('Create environments from Pull Requests'),
    ),
    '#attributes' => array(
      'class' => array('text small text-muted')
    )
  );

  $form['github']['devshop_github_token'] = array(
    '#type' => 'password',
    '#title' => t('GitHub Token'),
    '#description' => t('Enter a GitHub token to connect this DevShop to your code repositories. !link.', [
        '!link' => l(t('Create a new token'), 'https://github.com/settings/tokens/new?scopes=repo,admin:public_key,admin:repo_hook&description=' . $_SERVER['HTTP_HOST'], array('attributes' => array(
            'target' => '_blank',
        )))
    ]),
    '#default_value' => variable_get('devshop_github_token', ''),
    '#element_validate' => array(
      'devshop_github_settings_form_validate_token',
    ),
    '#attributes' => array(
      'placeholder' => $token? t('Replace the current GitHub Token'): t('Enter a GitHub Token'),
    ),
  );
  if (empty($token)) {
    $access_message = t('Token: not set up.');
    $access_class = 'info';
    $access_note = t('Enter a token to connect this DevShop to your GitHub repositories.');
  }
  elseif (!$token_valid) {
    $access_message = t('Token is invalid.');
    $access_class = 'danger';
    $access_note = t('The stored GitHub token is invalid. Enter a new one to continue.');
  }
  else {
    $access_message = t('Token is valid.');
    $access_class = 'success';
    $access_note = t('The stored GitHub token is valid.');
  }
  $form['github']['token_status'] = array(
    '#type' => 'item',
    '#title' => $access_message,
    '#prefix' => "<div class='alert alert-$access_class'>",
    '#description' => $access_note,
    '#suffix' => '</div>',
  );

  // SSH Key access
  if (empty($ssh_key)) {
    $access_message = t('Unknown Public Key');
    $access_class = 'warning';
    $access_note = t("DevShop does not know the user's public key. See DevShop Server Access: Public Key below.");
  }
  elseif (empty($token)) {
    $access_message = t('Public Key: Not set up.');
    $access_class = 'info';
    $access_note = t('Enter a token to check SSH access to your repositories.');
  }
  elseif (!devshop_github_check_key()) {
    $access_message = t('Public Key: Access Denied');
    $access_class = 'warning';
    $access_note = t("The Public Key was not found in the account that owns this GitHub token. Add the public key to individual repositories as a Deploy Key, or !link to have DevShop add it for you.", array(
      '!link' => l(t('Click Here'), 'admin/devshop/github/add-key', array(
          'query' => array(
              'destination' => $_GET['q'],
          ),
      ))));
  }
  else {
    $access_message = t('Public Key has access.');
    $access_class = 'success';
    $access_note = t("The server's Public Key was found in the GitHub account that owns this token. DevShop can clone your GitHub repositories using the Public Key stored below.");
  }
  $form['github']['key_status'] = array(
      '#type' => 'item',
      '#title' => $access_message,
      '#prefix' => "<div class='alert alert-$access_class'>",
      '#description' => $access_note,
      '#suffix' => '</div>',
  );

//
//  if (!$ssh_key_exists) {
//    $access_message = t("Unknown Public Key.");
//    $access_note = t("Cannot confirm access. DevShop does not know the server's public key. Enter it below.");
//    $access_class = 'warning';
//  }
//  elseif (!devshop_github_check_key()) {
//    $access_message = t("Confirmed");
//    $access_note = t("The Public Key below has access to your GitHub Repositories.");
//    $access_class = 'success';
//  }
//  elseif ($token_valid) {
//    $access_message = t('Access Denied');
//    $access_note = t("The Public Key below was not found in the GitHub account that owns this GitHub Token. !link or add it as a Deploy Key to your repository.", [
//        '!link' => l(t('Click here to add it to your GitHub account automatically'), 'admin/devshop/github/add-key', array(
//            'query' => array(
//                'destination' => $_GET['q'],
//            ),
//            'attributes' => array(
//                'class' => array('btn btn-info'),
//            ),
//        ))
//    ]);
//    $access_class = 'danger';
//  }

  $repos = variable_get('devshop_github_available_repositories', array());
  $count = count($repos);
  $form['projects']['repos'] = array(
    '#type' => 'container',
    '#weight' => 5,
    '#access' => !empty(variable_get('devshop_github_token')),
  );

  $form['github']['message'] = array(
    '#prefix' => "<div class='alert alert-success'><i class='fa fa-check'></i> ",
    '#suffix' => '</div>',
    '#markup' =>  t('@count_string associated with this GitHub token.', array(
      '@count_string' => format_plural($count, t('1 repository found'), t('@count repositories found'))
  )));

  $orgs = variable_get('devshop_github_available_organizations', array());
  $count = count($orgs);
  $form['github']['message_orgs'] = array(
    '#prefix' => "<div class='alert alert-success'><i class='fa fa-check'></i> ",
    '#suffix' => '</div>',
    '#markup' =>  t('@count_string with create repo privileges.', array(
      '@count_string' => format_plural($count, t('1 GitHub organization'), t('@count GitHub organizations'))
  )));
  $form['github']['button'] = array(
    '#markup' => l(t('Reload GitHub Projects'),'admin/devshop/github/load-repos', array('attributes' => array(
    'class' => array('btn btn-info'),
  ))));

  $form = system_settings_form($form);
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
  if (empty($token)) {
    form_set_value($element, variable_get('devshop_github_token'), $form_state);
    return;
  }
  $e = devshop_github_token_is_valid($token);
  if (is_subclass_of($e, 'Exception')) {
    form_error($element,  $e->getMessage());
  }
  else {
    form_set_value($element, $token, $form_state);
    devshop_github_refresh_repositories();
  }
}

/**
 * Page callback for "get all repos" link.
 */
function devshop_github_get_repositories_page() {
  $token = variable_get('devshop_github_token', '');
  if (empty($token)) {
    drupal_set_message('GitHub API Token is not set.', 'error');
    drupal_goto('admin/devshop');
    return;
  }

  devshop_github_refresh_repositories();

  drupal_goto('admin/devshop');
  return;
}

/**
 * Get a list of all repos a user can access.
 */
function devshop_github_refresh_repositories() {

  try {
    $client = devshop_github_client();
    $userApi = $client->currentUser();
    $orgsApi = $client->organizations();
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


    ;
    $params = array(
      'all'
    );
    $orgs = $paginator->fetchAll($userApi, 'organizations', $params);
    foreach ($orgs as $org) {

      // If membership role is admin, or organization ''''
      $membership = $userApi->memberships()->organization($org['login']);
      if ($membership['role'] == 'admin' || $orgsApi->show($org['login'])['members_can_create_repositories']) {
        $organizations[$org['login']] = $org['login'];
      }
    }
    $count = count($organizations);
    $r = array(
      '@count' => $count,
    );

    variable_set('devshop_github_available_organizations', $organizations);
    drupal_set_message(t('Found @count_string with create repo privileges.', array(
      '@count_string' => format_plural($count, '1 organization', t('@count organizations', $r)),
    )));


  }
  catch (\Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    drupal_set_message('Code ' . $e->getCode(), 'error');
  }
}
