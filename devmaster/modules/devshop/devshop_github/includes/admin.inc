<?php

/**
 * GitHub API key form.
 * @return array()
 */
function devshop_github_form_devshop_projects_settings_form_alter(&$form, &$form_state, $form_id) {

  $token = devshop_github_token();
  $token_valid = $token? devshop_github_token_is_valid($token): FALSE;

  $ssh_key = variable_get('devshop_public_key', '');
  $ssh_key_exists = !empty($ssh_key);

  $form['github'] = array(
    '#title' => t('GitHub Integration'),
    '#description' => '',
    '#type' => 'fieldset',
    '#weight' => -3,
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
  $hostmaster_platform = node_load(hosting_get_hostmaster_platform_nid());
  $hostmaster_server = node_load($hostmaster_platform->web_server);

  if (empty($token)) {
    $access_message = t('Not set up.');
    $access_class = 'info';
    $access_note = t('Enter a token to connect this DevShop to your GitHub repositories.');
  }
  elseif (!$token_valid) {
    $access_message = t('Invalid token.');
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

  $node = t('DevShop clones your code using the %app_user user on the server !hostmaster_server. !note', [
      '%app_user' => variable_get('devshop_app_user', 'aegir'),
      '!hostmaster_server' => l($hostmaster_server->title, "node/{$hostmaster_server->nid}"),
      '!note' => $access_note,
  ]);

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

  $form['projects']['message'] = array(
    '#prefix' => "<div class='alert alert-success'><i class='fa fa-check'></i>",
    '#suffix' => '</div>',
    '#weight' => -1,
    '#markup' =>  t('@count_string associated with this GitHub token.', array(
      '@count_string' => format_plural($count, t('1 repository found'), t('@count repositories found'))
  )));

  $orgs = variable_get('devshop_github_available_organizations', array());
  $count = count($orgs);
  $form['projects']['message_orgs'] = array(
    '#prefix' => "<div class='alert alert-success'><i class='fa fa-check'></i>",
    '#suffix' => '</div>',
    '#weight' => -1,
    '#markup' =>  t('@count_string with create repo privileges.', array(
      '@count_string' => format_plural($count, t('1 GitHub organization'), t('@count GitHub organizations'))
  )));
  $form['projects']['button'] = array(
    '#markup' => l(t('Refresh GitHub Data'),'admin/devshop/github/load-repos', array('attributes' => array(
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
    return;
  }
  $e = devshop_github_token_is_valid($token);
  if (is_subclass_of($e, 'Exception')) {
    form_error($element,  $e->getMessage());
  }
  else {
    variable_set('devshop_github_token', $token);
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
