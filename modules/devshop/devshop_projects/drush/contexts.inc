<?php

/**
 * Implements hook_hosting_project_context_options()
 *
 * This transfers data from the node to thes aegir context object (the alias!)
 * For project entities.  This is where we find the branches and tags on the remote.
 */
function devshop_projects_hosting_project_context_options(&$task) {


  $branches = getBranchesAndTags($task->ref->project->git_url);

  // If something went wrong connecting to the git repo, don't wipe out our branches.
  if (!empty($branches['branches'])){
    $task->ref->project->settings->git['branches'] = $branches['branches'];
    $task->ref->project->settings->git['tags'] = $branches['tags'];
    $task->ref->project->settings->git['refs'] = $branches['refs'];

    // Save the project node now that we have branches and tags.
    // Don't verify again, this is the verification process.
    $task->ref->no_verify = TRUE;
    node_save($task->ref);
  }

  // Save project object to drush alias (aegir context).
  if (isset($task->ref->project)) {
    $task->context_options['server'] = '@server_master';
    $task->context_options['project_name'] = $task->ref->title;
    $task->context_options['project'] = $task->ref->project;
  }
}

/**
 * Implements hook_hosting_site_context_options()
 *
 * Runs on verify task. Passes data to the drush alias.
 * Save environment name, project name, and git ref to site aliases.
 */
function devshop_projects_hosting_site_context_options(&$task) {

  if (isset($task->ref->environment)) {
    $task->context_options['environment'] = $task->ref->environment;
    $task->context_options['project'] = $task->ref->project->name;
    $task->context_options['git_ref'] = $task->ref->environment->git_ref;
  }
}

/**
 * Implements hook_drush_context_import()
 *
 * This allows project nodes to be created from contexts (aliases)
 */
function devshop_projects_drush_context_import($context, &$node) {
  if ($context->type == 'Project') {
    $node->title = $context->project_name;
    $node->type = 'project';
    $node->project = $context->project;
  }
}

/**
 * Helpfer for getting branches and tags from a git URL
 */
function getBranchesAndTags($git_url = NULL){
  if (is_null($git_url)){
    $git_url = drush_get_option('git_url');
  }
  $command = "git ls-remote {$git_url}";
  drush_log('[DEVSHOP] running '.$command, 'ok');

  if (drush_shell_exec($command)){
    $exec_output = drush_shell_exec_output();
  } else {
    $exec_output = drush_shell_exec_output();
    drush_set_error(DRUSH_FRAMEWORK_ERROR, dt('[DEVSHOP] Error retrieving remote information: ') . implode("\n", $exec_output), 'error');
    return;
  }


  // Check for Permission Denied
  // @TODO: Provide link to the Public key for the server.
  if ('Permission denied' == substr($exec_output[0], 0, 17)){
    drush_set_error(DRUSH_FRAMEWORK_ERROR, dt('[DEVSHOP] Error:'). implode("\n", $exec_output), 'error');
    return;
  }

  // If remote list is empty, something else went wrong.
  if (count($exec_output) == 1 && empty($exec_output[0])){
    drush_set_error(DRUSH_FRAMEWORK_ERROR, dt('Something went wrong.  Check the git URL and try again.'), 'error');
    return;
  }

  // Build tag and branch list
  $branches = array();
  $tags = array();
  $refs = array();

  foreach ($exec_output AS $line_string){

    // @TODO: Would love some regex love here
    // Example remote line:
    // 9fc5727c0823d8d3300ba5aae3328d5998033e45 refs/heads/3-00
    $line = trim(substr($line_string, 40));
    $ref = explode("/", $line);
    $git_ref = array_pop($ref);

    if ($ref[1] == 'heads') {
      $branches[] = $git_ref;
      $refs[$git_ref] = 'branch';
    } else if ($ref[1] == 'tags') {
      $tags[] = $git_ref;
      $refs[$git_ref] = 'tag';
    }

  }
  drush_log(dt('[DEVSHOP] Found !b branches and !t tags.', array('!b' => count($branches), '!t' => count($tags), )), 'ok');
  return array('branches' => $branches, 'tags' => $tags, 'refs' => $refs);
}