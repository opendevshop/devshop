<?php

/**
 * GitHub API key form.
 * @return array()
 */
function devshop_github_form_devshop_projects_settings_form_alter(&$form, &$form_state, $form_id) {

  $token = devshop_github_token();
  $client = new \Github\Client();
  $client->authenticate($token, \Github\AuthMethod::ACCESS_TOKEN);

  $repos = variable_get('devshop_github_available_repositories', array());
  $count = count($repos);

  try {
    $gh_user = $client->currentUser()->show();
    $token_valid = TRUE;
  }
    // Happens when user has no public keys.
  catch (Exception $e) {
    $gh_user = FALSE;
    $token_valid = FALSE;
  }

  $t = array(
    '!user' => $gh_user? l($gh_user['login'], $gh_user['html_url'], array(
      'attributes' => array(
        'target' => '_blank'))): '',
    '!create_token' => l(t('Create new token'), 'https://github.com/settings/tokens/new?scopes=repo,admin:public_key,admin:repo_hook&description=' . $_SERVER['HTTP_HOST'], array(
      'attributes' => array(
        'target' => '_blank'))),
    '!github_tokens' => l(t('GitHub Personal Tokens'), 'https://github.com/settings/tokens', array(
      'attributes' => array(
        'target' => '_blank'),
      'fragment' => 'edit-server')),
    '!github_keys' => l(t('GitHub SSH Keys'), 'https://github.com/settings/keys', array(
      'attributes' => array(
        'target' => '_blank'),
      'fragment' => 'edit-server')),
    '!devshop_settings' => l(t('DevShop Public Key'), current_path(), array(
      'fragment' => 'edit-server')),
    '!devshop_add_key' => l(t('Add DevShop Public Key to GitHub'), 'admin/hosting/github/add-key', array(
      'attributes' => array(
      ),
      'query' => array('destination' => current_path())))
  );

  $ssh_key = variable_get('devshop_public_key', '');

  $form['github'] = array(
    '#title' => t('GitHub Integration'),
    '#type' => 'fieldset',
    '#group' => 'settings',
    '#weight' => -3,
  );
  $form['github']['features']  = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array(
        'alert alert-info'
      )
    )
  );
  $form['github']['features']['list'] = array(
    '#theme' => 'item_list',
    '#prefix' => '<i class="fa fa-question-circle fa-2x pull-left"></i><div>' . t('DevShop integrates with GitHub to provide a number of features:'),
    '#suffix' => '</div>',
    '#items' => array(
      t('Create environments from Pull Requests'),
      t('Loads a list of available repositories for creating new projects.'),
      t('Create new repositories on GitHub.'),
      t('Post task statuses to GitHub to integrate with Deployments and Commit Status API.'),
    ),
    '#attributes' => array(
      'class' => array('text text-dark')
    )
  );

  $form['github']['devshop_github_token'] = array(
    '#type' => 'password',
    '#title' => t('GitHub Token'),
    '#description' => t('Enter a GitHub token to connect this DevShop to your repositories. <div class="btn-group">!create_token !github_tokens !github_keys</div>', $t),
    '#default_value' => variable_get('devshop_github_token', ''),
    '#element_validate' => array(
      'devshop_github_settings_form_validate_token',
    ),
    '#attributes' => array(
      'placeholder' => $token? t('Replace the current GitHub Token'): t('Enter a GitHub Token'),
    ),
  );
  $options = [];
  foreach ($repos as $repo) {
    $url = $repo['url'];
    $options[$repo['org']][$url] = $url;
  }
  $form['github']['github_available_repos'] = array(
    '#title' => t('Available GitHub Repositories'),
    '#description' => t('Select the repos to make available to users when creating projects.'),
    '#type' => 'select',
    '#multiple' => true,
    '#options' => $options,
    '#default_value' => variable_get('github_available_repos', array()),
    '#bootstrap_ignore_pre_render' => TRUE,
  );

  if (empty($token)) {
    $access_message = t('Token: not set up.');
    $access_class = 'warning';
    $access_icon = 'warning';
    $access_note = t('Enter a token to connect this DevShop to your GitHub repositories.');
  }
  elseif (!$token_valid) {
    $access_message = t('Token is invalid.');
    $access_class = 'danger';
    $access_icon = 'warning';
    $access_note = t('The stored GitHub token is invalid or has expired. Enter a new one to continue.');
  }
  else {
    $access_message = t('Token is valid.');
    $access_class = 'success';
    $access_icon = 'check';
    $access_note = t('Successfully connected to GitHub API as !user.', $t);
  }
  $form['github']['token_status'] = array(
    '#type' => 'item',
    '#title' => $access_message,
    '#prefix' => "<div class='alert alert-$access_class'><i class='fa fa-$access_icon pull-left'></i>",
    '#description' => $access_note,
    '#suffix' => '</div>',
  );

  // SSH Key access
  if (empty($ssh_key)) {
    $access_message = t('Unknown Public Key');
    $access_class = 'warning';
    $access_icon = 'check';
    $access_note = t("DevShop does not know the server's public key. See !devshop_settings.", $t);
  }
  elseif (empty($token)) {
    $access_message = t('Public Key: not set up.');
    $access_class = 'warning';
    $access_icon = 'warning';
    $access_note = t('Enter a token to check SSH access to your repositories.');
  }
  elseif (!$token_valid) {
    $access_message = t('Public Key: Invalid token.');
    $access_class = 'danger';
    $access_icon = 'warning';
    $access_note = t('Unable to check for SSH access without a valid token.');
  }
  elseif (!devshop_github_check_key()) {
    $access_message = t('Public Key: Not Found');
    $access_class = 'warning';
    $access_icon = 'warning';
    $access_note = t("The !devshop_settings was not found in account !user. You can: ", $t);
    $access_note .= theme('item_list', array(
      'type' => 'ol',
      'items' => array(
        t('Automatically !devshop_add_key.', $t),
        t('Manually add the !devshop_settings to your GitHub account on the !github_keys page.', $t),
        t('Add the !devshop_settings to each repository as a Deploy Key.', $t),
        t('Configure SSH manually on the server.', $t),
    )));
  }
  else {
    $access_message = t('Public Key has access.');
    $access_class = 'success';
    $access_icon = 'check';
    $access_note = t("The DevShop Public Key was found in !user's account.", $t);
  }
  $form['github']['key_status'] = array(
      '#type' => 'item',
      '#title' => $access_message,
      '#prefix' => "<div class='alert alert-$access_class'><i class='fa fa-$access_icon pull-left'></i>",
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

  $form['projects']['repos'] = array(
    '#type' => 'container',
    '#weight' => 5,
    '#access' => !empty(variable_get('devshop_github_token')),
  );

  $form['github']['message'] = array(
    '#prefix' => "<div class='alert alert-success'><i class='fa fa-check'></i> ",
    '#suffix' => '</div>',
    '#markup' =>  format_plural($count, t('1 repository found'), t('DevShop can access @count repositories.')));

  $orgs = variable_get('devshop_github_available_organizations', array());
  $count = count($orgs);
  $form['github']['message_orgs'] = array(
    '#prefix' => "<div class='alert alert-success'><i class='fa fa-check'></i> ",
    '#suffix' => '</div>',
    '#markup' =>  t('DevShop can create repositories in @count_string.', array(
      '@count_string' => format_plural($count, t('1 GitHub organization'), t('@count GitHub organizations'))
  )));
  $form['github']['button'] = array(
    '#access' => $token_valid,
    '#markup' => l(t('Refresh Repositories'), 'admin/hosting/github/load-repos', array(
      'query' => drupal_get_destination(),
      'attributes' => array(
        'class' => array('btn btn-default'),
      ))),
  );

  $form['create']['options']['devshop_allow_create_repo_github'] = array(
    '#title' => t('Allow creation of new repositories on GitHub.'),
    '#description' => t('Allow users to create new git repositories when starting a new project.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_allow_create_repo_github', TRUE),
  );
  $form['create']['devshop_composer_project_suggestions'] = array(
    '#title' => t('Available Composer Projects'),
    '#description' => t('When creating a new git repository, allow users to choose from these composer projects.'),
    '#type' => 'textarea',
    '#default_value' => variable_get('devshop_composer_project_suggestions', 'devshop/composer-template:8.x-dev', ''),
    '#states' => array(
      'visible' => array(
        ':input[name="devshop_allow_create_repo_github"]' => array('checked' => TRUE)
      ),
    ),
  );

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

  $client = new \Github\Client();
  $client->authenticate($token, \Github\AuthMethod::ACCESS_TOKEN);

  try {
    $gh_user = $client->currentUser()->show();
    drupal_set_message(t('Successully connected to GitHub API as !link.', array(
      '!link' => l($gh_user['login'], $gh_user['url']),
    )));
    return TRUE;
  }
    // Happens when user has no public keys.
  catch (Exception $e) {
    form_error($element,  t('GitHub Token was rejected: %message', array(
      '%message' => $e->getMessage(),
    )));
  }
}

/**
 * Page callback for "get all repos" link.
 */
function devshop_github_get_repositories_page() {
  $token = variable_get('devshop_github_token', '');
  if (empty($token)) {
    drupal_set_message('GitHub API Token is not set.', 'error');
    drupal_goto('admin/hosting');
    return;
  }

  devshop_github_refresh_repositories();

  drupal_goto('admin/hosting');
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
