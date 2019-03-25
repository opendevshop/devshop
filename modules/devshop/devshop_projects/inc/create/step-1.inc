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
      '#markup' => t('We were unable to connect to your git repository. Check the messages, edit your settings, and try again.'),
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
    );
  }
  
  $username = variable_get('aegir_user', 'aegir');

  $tips[] = t('For best results, use the !link project as a starting point for Composer based projects.', [
    '!link' => l(t('DevShop Composer Template'), 'https://github.com/opendevshop/devshop-composer-template', array(
      'attributes' => array(
        'target' => '_blank',
      )
    )),
  ]);

  $tips[] = t('Use the "ssh" url whenever possible. For example: <code>git@github.com:opendevshop/repo.git</code>');
  $tips[] = t('You can use a repository with a full Drupal stack committed, in the root or in a sub folder');
  $tips[] = t('If a composer.json file is found in the root of the project, <code>composer install</code> will be run automatically.');
  $tips[] = t('If your repository is not public, make sure your server has SSH access. Click the "Repository Access" button below to see the SSH key of this DevShop.');
  $tips = theme('item_list', array('items' => $tips));
  
  $form['git_url'] = array(
    '#type' => 'textfield',
    '#required' => 1,
    '#title' => t('Git URL'),
    '#suffix' => '<p>' . t('Enter the Git URL for your project') . '</p><div class="alert alert-info">' . $tips . "</div>",
    '#default_value' => $project->git_url,
    '#maxlength' => 1024,
    '#weight' => -1,
    '#attributes' => array(
      'placeholder' => 'git@githost.com:myteam/myproject.git'
    ),
  );

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
  if (empty($pubkey)) {
    $output = t("This DevShop doesn't yet know your server's public SSH key. To import it, run the following command on your server as <code>aegir</code> user:");
    $command = 'drush @hostmaster vset devshop_public_key "$(cat ~/.ssh/id_rsa.pub)" --yes';
    $output .= "<div class='command'><input size='160' value='$command' onclick='this.select()' /></div>";
  }
  else {
    // @TODO: Make this Translatable
    $output = <<<HTML
  <div class="empty-message">If you haven't granted this server access to your Git repository, you should do so now using it's public SSH key.</div>
  <textarea id="rsa" onclick='this.select()'>$pubkey</textarea>
HTML;
  }
  
  // Add info about connecting to Repo
  $form['connect'] = array(
    '#type' => 'fieldset',
    '#title' => '<i class="fa fa-chevron-right"></i> ' . t('Repository Access Information'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['connect']['#description'] = $output;
  return $form;
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
  $project_name = strtolower(preg_replace('/[^a-z0-9]/gi', '', $element['value']));
  form_set_value($form['title'], $project_name, $form_state);
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
  
//
//  // CODE PATH & BASE URL
//  // Code path and base url must be unique
//  if (db_query('SELECT n.nid FROM {hosting_devshop_project} d LEFT JOIN {node} n ON d.nid = n.nid WHERE status = :status AND code_path = :code_path;', array(':status' => 1, ':code_path' => $form_state['values']['code_path']))->fetchField()) {
//    form_set_error('code_path', t('Another project already has this code path.  Code path must be unique.'));
//  }
//  if (db_query('SELECT n.nid FROM {hosting_devshop_project} d LEFT JOIN {node} n ON d.nid = n.nid WHERE status = :status AND base_url = :base_url;', array(':status' => 1, ':base_url' => $form_state['values']['base_url']))->fetchField()) {
//    form_set_error('base_url', t('Another project already has this base url.  Base URL must be unique.'));
//  }
//
//  // If custom code paths are not allowed, set the value here using the project name.
//  if (!variable_get('devshop_projects_allow_custom_code_path', FALSE)) {
//    form_set_value($form['code_path'], variable_get('devshop_project_base_path', '/var/aegir/projects') . '/' . $project_name, $form_state);
//  }
//
//  // If custom base URLs are not allowed, set the value here using the project name.
//  if (!variable_get('devshop_projects_allow_custom_base_url', FALSE)) {
//    form_set_value($form['base_url'], devshop_projects_url($project_name), $form_state);
//  }
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
  drupal_get_messages();
}
