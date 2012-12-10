<?php
/**
 * @file devshop_project.task.inc DevShop Project Task related hooks and
 * support function include file.
 */

/**
 * Implementation of hook_hosting_tasks()
 */
function devshop_projects_hosting_tasks() {
  $tasks = array();
  $tasks['project']['verify'] = array(
    'title' => t('Verify Project'),
    'description' => t('Verifies access to the git repository, downloads branch and tag information.'),
    'provision_save' => TRUE,
    'access callback' => 'devshop_hosting_task_menu_access',
  );
  $tasks['project']['devshop-create'] = array(
    'title' => t('Create New Platform'),
    'description' => t('Creates a new platform within this project.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
  );
  $tasks['project']['delete'] = array(
    'title' => t('Delete Project'),
    'description' => t('Delete a project and all associated sites and platforms.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
  );
  return $tasks;
}

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "devshop-create" task for creating a platform and site.
 */
function hosting_task_devshop_create_form($node) {
  $branch_options = array_combine($node->git_branches, $node->git_branches);
  $form['branch'] = array(
    '#title' => t('Branch'),
    '#description' => t('Choose the Git branch you with to use for this new platform.  <em>Note: If you do not see all remote branches, re-verify your project.</em>'),
    '#type' => 'select',
    '#options' => $branch_options,
    '#required' => TRUE,
  );
  // @TODO: Add "create branch" functionality.
  $form['platform_name'] = array(
    '#title' => t('Platform Name'),
    '#type' => 'textfield',
    '#description' => t('Enter the system name of your platform.  For consistency, you should make this match the branch name.'),
    '#required' => TRUE,
  );
  /* @TODO: Let devshop_pull handle this.
  $form['pull'] = array(
    '#title' => t('Enable Pull on Commit'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  */
  $form['#submit'][] = 'hosting_task_devshop_create_form_submit';
  return $form;
}

/**
 * Extra submit function for hosting_task_confirm_form()
 *
 * @see devshop_projects_form_alter().  We had to add the submit hadler there.
 */
function hosting_task_devshop_create_form_submit($form, &$form_state) {

  $project = node_load($form_state['values']['nid']);
  $platform_name = $form_state['values']['parameters']['platform_name'];
  $branch = $form_state['values']['parameters']['branch'];
  $servers = hosting_get_servers('http');
  $server = variable_get('devshop_projects_default_web_server', key($servers));

  // hosting_platform fields
  $platform = new stdClass;
  $platform->title = $project->title . '_' . $platform_name;
  $platform->publish_path = $project->code_path . '/' . $platform_name;
  $platform->web_server = $server;
  $platform->git_branch = $branch;
  $platform->project = $project->title;
  $platform->environment = $platform_name;

  watchdog('debug', 'Form state: ' . print_r($form_state['values'],1));

  watchdog('debug', 'Attempting to create: ' . print_r($platform,1));

  $platform_node = _devshop_projects_node_create('platform', $platform);


}
/**
 * Implements hook_hosting_project_context_options()
 *
 * This transfers data from the node to thes aegir context object (the alias!)
 * For project entities.  This is where we find the branches and tags on the remote.
 */
function devshop_projects_hosting_project_context_options(&$task) {

  drush_log('[DEVSHOP] devshop_projects_hosting_project_context_options()', 'ok');
  $task->context_options['server'] = '@server_master';
  $task->context_options['project_name'] = $task->ref->title;
  $task->context_options['install_profile'] = $task->ref->install_profile;
  $task->context_options['base_url'] = $task->ref->base_url;
  $task->context_options['code_path'] = trim($task->ref->code_path, " ");
  $task->context_options['git_url'] = $task->ref->git_url;

  $branches = getBranchesAndTags($task->ref->git_url);

  $task->ref->git_branches = $branches['branches'];
  $task->ref->git_tags = $branches['tags'];
  node_save($task->ref);

  $task->context_options['git_branches'] = $branches['branches'];
  $task->context_options['git_tags'] = $branches['tags'];
}

/**
 * Helpfer for getting branches and tags from a git URL
 */
function getBranchesAndTags($git_url = NULL){
  if (is_null($git_url)){
    $git_url = drush_get_option('git_url');
  }
  $command = "git ls-remote {$git_url}";
  drush_shell_exec($command);
  $exec_output = drush_shell_exec_output();

  drush_log('[DEVSHOP] running '.$command, 'ok');
  drush_log('[DEVSHOP] ' . implode("\n", $exec_output), 'ok');

  // Check for Permission Denied
  // @TODO: Provide link to the Public key for the server.
  if ('Permission denied' == substr($exec_output[0], 0, 17)){
    drush_set_error(DRUSH_FRAMEWORK_ERROR, dt('[DEVSHOP] Error:'). implode("\n", $exec_output), 'error');
  }

  // If remote list is empty, something else went wrong.
  if (count($exec_output) == 1 && empty($exec_output[0])){
    drush_set_error(DRUSH_FRAMEWORK_ERROR, dt('Something went wrong.  Check the git URL and try again.'), 'error');
    return;
  }

  // Build tag and branch list
  $branches = array();
  $tags = array();

  foreach ($exec_output AS $line_string){
    // @TODO: Would love some regex love here
    // Example remote line:
    // 9fc5727c0823d8d3300ba5aae3328d5998033e45	refs/heads/3-00
    $line = explode("	", $line_string);
    $ref = explode("/", $line[1]);

    $branch = array_pop($ref);

    if ($ref[1] == 'heads') {
      $branches[] = $branch;
    } else if ($ref[1] == 'tags') {
      $tags[] = $branch;
    }
  }
  drush_log(dt('[DEVSHOP] Found !b branches and !t tags.', array('!b' => count($branches), '!t' => count($tags), )), 'ok');
  return array('branches' => $branches, 'tags' => $tags);
}

/**
 * Implements hook_hosting_site_context_options()
 *
 * This transfers data from the node to the aegir context object (the alias!)
 * For site entities.
 */
function devshop_projects_hosting_site_context_options(&$task) {
  $task->context_options['project'] = $task->ref->project;
  $task->context_options['nerd'] = 'vision';
}

/**
 * Implements hook_hosting_site_context_options()
 *
 * This transfers data from the node to the aegir context object (the alias!)
 * For site entities.
 */
function devshop_projects_hosting_platform_context_options(&$task) {
  if (!empty($task->ref->project)){
    $task->context_options['project'] = $task->ref->project;
    $task->properties['task properties'] = 'works';
    $task->ref->properties['task ref properties'] = 'works';

    d()->setProperty('setProperty', 'works');
  }
}

