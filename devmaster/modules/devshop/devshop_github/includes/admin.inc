<?php

/**
 * GitHub API key form.
 * @return array()
 */
function devshop_github_form_alter(&$form, &$form_state, $form_id) {

  // @TODO: form_FORM_ID_alter
  if ($form_id != 'devshop_projects_settings_form') {
    return;
  }

  $token = devshop_github_token();

  $token_exists = !empty($token);

  $ssh_key = variable_get('devshop_public_key', '');
  $ssh_key_exists = !empty($ssh_key);

  $form['devshop_github_token_step'] = array(
    '#title' => t('GitHub Integration'),
    '#description' => '',
    '#type' => 'fieldset',
    '#weight' => -3,
  );

  $form['devshop_github_token_step']['description'] = array(
    '#prefix' => '<div class="alert alert-info">',
    '#markup' =>
      '<p>' . t('Connect GitHub to your DevShop Server by creating a Github token and adding it here.') . '<p>' .
      '<p>' . l(t('Create a new GitHub API Token'), 'https://github.com/settings/tokens/new?scopes=repo,admin:public_key,admin:repo_hook&description=' . $_SERVER['HTTP_HOST'], array('attributes' => array(
        'target' => '_blank',
        'class' => array('btn btn-sm btn-default form-submit'),
      ))),
    '#suffix' => '</div>',
  );

  $form['devshop_github_token_step']['message'] = array(
    '#markup' => $token_exists? '<i class="fa fa-check"></i> ' . t('The saved GitHub Token is valid. This DevShop can connect to the GitHub API.'): '',
    '#prefix' => '<div class="alert alert-success">',
    '#suffix' => '</div>',
    '#access' => $token_exists
  );

  $form['devshop_github_token_step']['devshop_github_token'] = array(
    '#type' => 'password',
    '#description' => t('Copy the token you created on GitHub.com and paste it here.'),
    '#default_value' => variable_get('devshop_github_token', ''),
    '#element_validate' => array(
      'devshop_github_settings_form_validate_token',
    ),
    '#attributes' => array(
      'placeholder' => $token_exists? t('Enter a new GitHub Token'): t('Enter a GitHub Token'),
    ),
    '#weight' => 2,
  );

  $form['server']['devshop_github_ssh_key'] = array(
    '#weight' => -1,
  );

  $form['server']['devshop_github_ssh_key']['ssh_key_status'] = array(
  );

  if (!$token_exists) {
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#prefix'] = ' <div class="text ">';
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#suffix'] = '</div>';
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#markup'] = t('You must enter a GitHub token before SSH access can be setup and confirmed.');
  }
  elseif (!$ssh_key_exists) {
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#prefix'] = ' <div class="alert alert-warning">';
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#suffix'] = '</div>';
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#markup'] = t('DevShop does not know what this server\'s public SSH Key is. Run the following command on the server to import the aegir user\'s public key <textarea class="form-control" onclick="this.focus();this.select()" >drush @hostmaster vset devshop_public_key "$(cat ~/.ssh/id_rsa.pub)" --yes</textarea>');
  }
  elseif (devshop_github_check_key()) {
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#prefix'] = ' <div class="alert alert-success">';
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#suffix'] = '</div>';
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#markup'] = '<i class="fa fa-check"></i>' . t("This DevShop's Public Key was found in the GitHub account associated with the GitHub Token.");
  }
  else {
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#markup'] = t("This DevShop's Public Key was not found in your GitHub account.");
    $form['server']['devshop_github_ssh_key']['ssh_key_status_buttons'] ['#markup'] .= '<div class="btn-group" role="group">' . l(t('Add public key to your GitHub account'), 'admin/devshop/github/add-key', array(
        'query' => array(
          'destination' => $_GET['q'],
        ),
        'attributes' => array(
          'class' => array('btn btn-info'),
        ),
      ));
    $form['server']['devshop_github_ssh_key']['ssh_key_status_buttons'] ['#markup'] .= l(t('Configure Public Key'), 'admin/devshop', array(
        'query' => array(
          'destination' => $_GET['q'],
        ),
        'attributes' => array(
          'class' => array('btn btn-default')
        ),
      )) . '</div>';
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#prefix'] = '<div class="alert alert-warning">';
    $form['server']['devshop_github_ssh_key']['ssh_key_status'] ['#suffix'] = '</div>';
  }

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
