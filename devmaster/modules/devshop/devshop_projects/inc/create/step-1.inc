<?php

/**********
 * STEP 1
 * Git URL
 **********/

/**
 * STEP 1: Form
 *
 * Just get the Git URL.  If we can't access the code, all else should fail.
 */
function devshop_project_create_step_git($form, $form_state) {
  $project = &$form_state['project'];
  
  drupal_add_js(drupal_get_path('module', 'devshop_projects') . '/inc/create/create.js');
  
  if ($project->verify_error) {
    $form['note'] = array(
      '#markup' => t('Something went wrong. Check the errors and settings and try again.'),
      '#prefix' => '<div class="alert alert-danger">',
      '#suffix' => '</div>',
      '#weight' => -999,
    );
    $form['error'] = array(
      '#markup' => $project->verify_error,
      '#weight' => -998,
    );
    
    // Check for "host key"
    if (strpos($project->verify_error, 'Host key verification failed')) {
      $form['help'] = array(
        '#markup' => t('Looks like you need to authorize this host.  SSH into the server as <code>aegir</code> user and run the command <code>git ls-remote !repo</code>. <hr />Add <code>StrictHostKeyChecking no</code> to your <code>~/.ssh/config</code> file to avoid this for all domains in the future.', array(
          '!repo' => $project->git_url,
        )),
        '#prefix' => '<div class="alert alert-warning">',
        '#suffix' => '</div>',
      );
    }
  }
  
  if (empty($project->name)) {
    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Project Code Name'),
      '#required' => TRUE,
      '#suffix' => '<p>' . t('Choose a unique name for your project. Only lowercase letters and numbers are allowed. No punctuation, spaces or special characters.') . '</p>',
      '#size' => 40,
      '#maxlength' => 255,
      '#attributes'=> array(
        'placeholder' => t('myproject')
      ),
      '#element_validate' => array(
        'devshop_project_wizard_title_validate',
      ),
      '#weight' => 1,
    );
  }
  else {
    $form['title'] = array(
      '#type' => 'value',
      '#value' => $project->name,
    );
    $form['title_display'] = array(
      '#type' => 'item',
      '#title' => t('Project Code Name'),
      '#markup' => $project->name,
      '#weight' => 1,
    );
  }
  
  $username = variable_get('aegir_user', 'aegir');

//
//  // Project code path.
//  $form['code_path'] = array(
//    '#type' => variable_get('devshop_projects_allow_custom_code_path', FALSE) ? 'textfield' : 'value',
//    '#title' => t('Code path'),
//    '#description' => t('The absolute path on the filesystem that will be used to create all platforms within this project. There must not be a file or directory at this path.'),
//    '#required' => variable_get('devshop_projects_allow_custom_code_path', FALSE),
//    '#size' => 40,
//    '#default_value' => $project->code_path,
//    '#maxlength' => 255,
//    '#attributes' => array(
//      'data-base_path' => variable_get('devshop_project_base_path', '/var/aegir/projects'),
//    ),
//  );
//
//  // Project base url
//  $form['base_url'] = array(
//    '#type' => variable_get('devshop_projects_allow_custom_base_url', FALSE) ? 'textfield' : 'value',
//    '#title' => t('Base URL'),
//    '#description' => t('All sites will be under a subdomain of this domain.'),
//    '#required' => variable_get('devshop_projects_allow_custom_base_url', FALSE),
//    '#size' => 40,
//    '#default_value' => $project->base_url,
//    '#maxlength' => 255,
//    '#attributes' => array(
//      'data-base_url' => devshop_projects_url($project->name, ''),
//    ),
//  );
  
  // Display helpful tips for connecting.
  $pubkey = variable_get('devshop_public_key', '');
  
  // If we don't yet have the server's public key saved as a variable...
  if (empty($pubkey) && drupal_valid_path('admin/hosting')) {
    drupal_set_message(t("DevShop can display it's Public Key for you when creating projects, to make granting access to your git repositories easier. However, this DevShop does not know the it's public key. To fix this, check !settings. ", [
      '!settings' => l(t('DevShop Settings: Public Key'), 'admin/hosting', ['fragment' => 'edit-public-key']),
    ]), 'warning');
  }

  $tips[] = t('For best results, use the !link project as a starting point for Composer based projects.', [
    '!link' => l(t('DevShop Composer Template'), 'https://github.com/opendevshop/devshop-composer-template', array(
      'attributes' => array(
        'target' => '_blank',
      )
    )),
  ]);

  $form['welcome'] = array(
    '#markup' => '<h2 class="title">' . t("Start a new DevShop Project.") . '</h2><p>' . t("A project represents your website. DevShop keeps track of the source code for your website so it can launch copies of it quickly.") . '</p>',
    '#prefix' => '<div class="alert alert-success alert-project-welcome">',
    '#suffix' => '</div>',
    '#weight' => -1000,
  );

  $options = module_invoke_all('devshop_project_git_repo_options');
  $form['source'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array(
        'well'
      ),
    ),
  );
  $form['source']['git_source'] = array(
    '#title' => t('Choose the source of your code'),
    '#type' => count($options) == 1? 'value': 'radios',
    '#options' => $options,
    '#description' => t(''),
    '#default_value'=> key($options),
  );

  $form['source']['git_url'] = array(
    '#access' => !empty($options['custom']),
    '#type' => 'textfield',
    '#required' => 1,
    '#title' => t('Git Repository URL'),
    '#default_value' => $project->git_url,
    '#maxlength' => 1024,
    '#attributes' => array(
      'placeholder' => 'git@githost.com:myteam/myproject.git'
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="git_source"]' => array(
          'value' => 'custom'
        ),
      ),
    ),
  );

  $repos = explode(PHP_EOL, variable_get('devshop_project_create_choice_options', 'git@github.com:opendevshop/drupal-project.git'));
  foreach ($repos as $repo_name => $remote) {
    $repo_options[$remote] = $remote;
  }

  $form['source']['choice'] = array(
    '#access' => !empty($options['custom']),
    '#type' => 'select',
    '#title' => t('Select a git repository.'),
    '#options' => $repo_options,
    '#states' => array(
      'visible' => array(
        ':input[name="git_source"]' => array(
          'value' => 'choice'
        ),
      ),
    ),
  );

  // We have to tell the front-end what type of field so we can assign the right event.
  drupal_add_js(array(
    'devshop' => array(
      'projectNameSourceElements' => array(
        '#edit-git-url'
      ),
    ),
  ), 'setting');

  $tips[] = t('Use the "ssh" url whenever possible. For example: <code>git@github.com:opendevshop/repo.git</code>');
  $tips[] = t('You can use a repository with a full Drupal stack committed, in the root or in a sub folder');
  $tips[] = t('If a composer.json file is found in the root of the project, <code>composer install</code> will be run automatically.');
  $tips[] = t("If your repository is not public, make sure your server has SSH access. Check the !link to see this server's public key.", array(
    '!link' => l(t('DevShop Settings'), 'admin/hosting'),
  ));
  $tips = theme('item_list', array('items' => $tips));


  $form['tips'] = array(
      '#markup' => $tips,
      '#prefix' => '<div class="alert alert-info">',
      '#suffix' => '</div>',
    '#weight' => 10,
  );

  return $form;
}

/**
 * Implements hook_devshop_project_git_repo_options().
 */
function devshop_projects_devshop_project_git_repo_options() {
  return array(
    'custom' => t('Git repository URL'),
    'choice' => t('Choose a repository'),
  );
}


/**
 * Force the project to be lowercase and remove any non numbers or letters.
 *
 * @param $element
 * @param $form_state
 * @param $form
 */
function devshop_project_wizard_title_validate($element, &$form_state, $form) {
  // No spaces or special characters allowed.
  // Set the value to our trimmed and lowercased project name.
  $project_name = strtolower(preg_replace('/[^a-z0-9]+/', '', $element['#value']));
  form_set_value($form['title'], $project_name, $form_state);

  // Mention name was changed
  if ($project_name != $element['#value']) {
    drupal_set_message(t('Project name was converted to %name, because it cannot contain any spaces or non-alphanumeric characters.', array(
      '%name' => $project_name,
    )), 'warning');
  }

  // If still contains spaces, throw error.
  if (strpos($project_name, ' ') !== FALSE) {
    form_set_error('title', t('Project name cannot contain any spaces or non-alphanumeric characters.'));
  }
}

/**
 * STEP 1: Validate
 */
function devshop_project_create_step_git_validate(&$form, &$form_state) {
  $project = &$form_state['project'];
  
  if (empty($project->nid)) {

    $project_name = $form_state['values']['title']; // domain names are case-insensitive

    // Check for duplicate project name here.
    // hosting_context_load() only works if a node exists with that ID.
    if (db_query("SELECT nid FROM {hosting_context} WHERE name = :name", array(':name' => 'project_' . $project_name))->fetch()) {
      form_set_error('title', t('The name @project is in use.  Please try again.', array(
        '@project' => $project_name,
      )));
    }
  }

  // Set git_url from choice.
  if ($form_state['values']['git_source'] == 'choice' && !empty($form_state['values']['choice'])) {
    form_set_value($form['source']['git_url'], $form_state['values']['choice'], $form_state);
  }
}

/**
 * STEP 1: Submit
 */
function devshop_project_create_step_git_submit(&$from, &$form_state) {
  global $user;
  
  $project = &$form_state['project'];
  
  // If the project already exists, this means the git url has changed...
  if ($project->nid) {
    // Change the git url and save the node.  Verification will run again.
    $node = node_load($project->nid);
    $node->project->git_url = $form_state['values']['git_url'];
    $node->project->code_path = $form_state['values']['code_path'];
    $node->project->base_url = $form_state['values']['base_url'];

    $project->settings->create_project_settings = $form_state['values']['settings'];

    node_save($node);
  }
  // Create new project node
  elseif (empty($project->nid)) {
    // Create the project node now.  We will update it with the chosen path.
    // This is so we can check for branches and save the hosting_context as soon
    // as possible.
    $node = new stdClass;
    $node->title = $form_state['values']['title'];
    
    $node->type = 'project';
    $node->status = 0;
    $node->uid = $user->uid;
    $node->name = $user->name;
    
    // Create project object
    $project->name = $form_state['values']['title'];
    $project->git_url = $form_state['values']['git_url'];
    $project->code_path = $form_state['values']['code_path'];
    $project->base_url = $form_state['values']['base_url'];

    $project->settings = new stdClass();
    $project->settings->create_project_settings = $form_state['values']['settings'];

    // @TODO: We will clone the code for step 2 and look for drupal.
    $project->drupal_path = variable_get('devshop_projects_default_drupal_path', '');
    
    // Attach to node
    $node->project = $project;
    
    // Save project node, triggering verification.
    if ($node = node_submit($node)) {
      node_save($node);
    }
    
    // Save NID to ctools object cache.
    if ($node->nid) {
      $project->nid = $node->nid;
    }
  }

  // Remove default "task" messages.
  // drupal_get_messages();
}
