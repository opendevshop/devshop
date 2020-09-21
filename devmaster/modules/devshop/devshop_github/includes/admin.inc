<?php

/**
 * GitHub API key form.
 * @return array()
 */
function devshop_github_settings_form($form, &$form_state) {
  $token = devshop_github_token();

  $token_exists = !empty($token);

  $ssh_key = variable_get('devshop_public_key', '');
  $ssh_key_exists = !empty($ssh_key);

  $form['#prefix'] = '<div class="jumbotron">';
  $form['#suffix'] = '</div>';

  $form['title'] = array(
    '#markup' => t('GitHub Settings'),
    '#prefix' => '<h3>',
    '#suffix' => '</h3>',
  );
  $form['description'] = array(
    '#prefix' => '<p class="text-muted">',
    '#markup' => $token_exists?
      '':
      t('There is no GitHub token stored in this DevShop. Follow the steps below.'),
    '#suffix' => '</p>',
  );
  $form['devshop_github_token_button'] = array(
    '#type' => 'item',
    '#prefix' => '<div class="">',
    '#suffix' => '</div>',
    '#title' => t('Step 1: Create a GitHub Token'),
    '#markup' => '<div>' . l(t('Create a new token on GitHub.com'), 'https://github.com/settings/tokens/new?scopes=repo,admin:public_key,admin:repo_hook&description=' . $_SERVER['HTTP_HOST'], array('attributes' => array(
      'target' => '_blank',
      'class' => array('btn btn-info form-submit'),
    ))) . '</div>',
    '#description' => t('Click this button to take you to GitHub.com and pre-set the required scopes for a new Personal Access Token.'),
    '#weight' => 1,
  );

  $form['devshop_github_token_step'] = array(
    '#title' => t('Step 2: Enter your GitHub Token'),
    '#description' => t('If left blank, DevShop will attempt to use the Public API.'),
    '#type' => 'item',
    '#weight' => 2,
  );

  $form['devshop_github_token_step']['message'] = array(
    '#markup' => $token_exists? '<i class="fa fa-check"></i> ' . t('The stored GitHub Token is valid.'): '',
    '#prefix' => '<div class="alert alert-success">',
    '#suffix' => '</div>',
    '#access' => $token_exists
  );
  $form['devshop_github_token'] = array(
    '#type' => 'password',
    '#description' => $token_exists?
      t('If you wish to change it, enter a new one and press "Save Token".'):
      t('Copy the token you created on GitHub.com and paste it here.'),
    '#default_value' => variable_get('devshop_github_token', ''),
    '#element_validate' => array(
      'devshop_github_settings_form_validate_token',
    ),
    '#attributes' => array(
      'placeholder' => $token_exists?
        t('Token is saved.'):
        t('Enter a new GitHub Token')
    ),
    '#weight' => 2,
  );

  $form['devshop_github_ssh_key'] = array(
    '#title' => t('Step 3: Add the DevShop SSH Key to your account'),
    '#type' => 'item',
    '#weight' => 4,
  );

  $form['devshop_github_ssh_key']['ssh_key_status'] = array(

  );

  if (!$token_exists) {
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#prefix'] = ' <div class="text ">';
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#suffix'] = '</div>';
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#markup'] = t('You must enter a GitHub token before SSH access can be setup and confirmed.');
  }
  elseif (!$ssh_key_exists) {
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#prefix'] = ' <div class="alert alert-warning">';
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#suffix'] = '</div>';
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#markup'] = t('DevShop does not know what this server\'s public SSH Key is. Run the following command on the server to import the aegir user\'s public key <textarea class="form-control" onclick="this.focus();this.select()" >drush @hostmaster vset devshop_public_key "$(cat ~/.ssh/id_rsa.pub)" --yes</textarea>');
  }
  elseif (devshop_github_check_key()) {
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#prefix'] = ' <div class="alert alert-success">';
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#suffix'] = '</div>';
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#markup'] = t("This DevShop's public SSH Key is in your GitHub account.");
  }
  else {
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#markup'] = t("This DevShop's public SSH Key was not found in your GitHub account.");
    $form['devshop_github_ssh_key']['ssh_key_status_buttons'] ['#markup'] .= '<div class="btn-group" role="group">' . l(t('Add public key to your GitHub account'), 'admin/devshop/github/add-key', array(
        'query' => array(
          'destination' => $_GET['q'],
        ),
        'attributes' => array(
          'class' => array('btn btn-info'),
        ),
      ));
    $form['devshop_github_ssh_key']['ssh_key_status_buttons'] ['#markup'] .= l(t('Configure Public Key'), 'admin/devshop', array(
        'query' => array(
          'destination' => $_GET['q'],
        ),
        'attributes' => array(
          'class' => array('btn btn-default')
        ),
      )) . '</div>';
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#prefix'] = '<div class="alert alert-warning">';
    $form['devshop_github_ssh_key']['ssh_key_status'] ['#suffix'] = '</div>';
  }

  $repos = variable_get('devshop_github_available_repositories', array());
  $count = count($repos);
  $form['repos'] = array(
    '#type' => 'container',
    '#weight' => 5,
    '#access' => !empty(variable_get('devshop_github_token')),
  );

  $form['repos']['message'] = array(
    '#prefix' => "<div class='alert alert-info'>",
    '#suffix' => '</div>',
    '#markup' =>  t('@count_string associated with this GitHub token.', array(
      '@count_string' => format_plural($count, t('1 repository found'), t('@count repositories found'))
  )));

  $orgs = variable_get('devshop_github_available_organizations', array());
  $count = count($orgs);
  $form['repos']['message_orgs'] = array(
    '#prefix' => "<div class='alert alert-info'>",
    '#suffix' => '</div>',
    '#markup' =>  t('@count_string with create repo privileges.', array(
      '@count_string' => format_plural($count, t('1 GitHub organization'), t('@count GitHub organizations'))
  )));
  $form['repos']['button'] = array(
    '#markup' => l(t('Refresh GitHub Data'),'admin/devshop/github/load-repos', array('attributes' => array(
    'class' => array('btn btn-info'),
  ))));

  $form = system_settings_form($form);
  $form['actions']['submit']['#value'] = t('Save Token');
  $form['actions']['#weight'] = 3;
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
