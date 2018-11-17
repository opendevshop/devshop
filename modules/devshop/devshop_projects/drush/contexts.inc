<?php

/**
 * Implements hook_hosting_project_context_options().
 *
 * This transfers data from the node to thes aegir context object (the alias!)
 * For project entities.  This is where we find the branches and tags on the remote.
 */
function devshop_projects_hosting_project_context_options(&$task) {


  $branches = getBranchesAndTags($task->ref->project->git_url);

  // If something went wrong connecting to the git repo, don't wipe out our branches.
  if (!empty($branches['branches'])) {
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
 * Implements hook_hosting_site_context_options().
 *
 * Runs on verify task. Passes data to the drush alias.
 * Save environment name, project name, and git ref to site aliases.
 */
function devshop_projects_hosting_site_context_options(&$task) {

  if (isset($task->ref->environment)) {
    $task->context_options['environment'] = $task->ref->environment->name;
    $task->context_options['environment_settings'] = $task->ref->environment->settings;
    $task->context_options['project'] = $task->ref->project->name;
    $task->context_options['git_ref'] = $task->ref->environment->git_ref;
  }

  // If install task is set to force-reinstall, pass to drush options.
  if ($task->task_type == 'install' && $task->task_args['force-reinstall']) {
    $task->options['force-reinstall'] = 1;
  }

}

/**
 * Implements hook_hosting_site_context_options().
 *
 * Runs on verify task. Passes data to the drush alias.
 * Save environment name, project name, and git ref to site aliases.
 */
function devshop_projects_hosting_platform_context_options(&$task) {

  if (isset($task->ref->environment)) {
    $task->context_options['environment'] = $task->ref->environment->name;
    $task->context_options['project'] = $task->ref->project->name;
    $task->context_options['git_ref'] = $task->ref->environment->git_ref;
  }
}

/**
 * Implements hook_drush_context_import().
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
function getBranchesAndTags($git_url = NULL) {
  if (is_null($git_url)) {
    $git_url = drush_get_option('git_url');
  }
  $command = "git ls-remote {$git_url}";
  $output = provision_process($command);
  $lines = explode("\n", $output);

  // Build tag and branch list
  $branches = array();
  $tags = array();
  $refs = array();

  foreach ($lines as $line_string) {

    // "annotated" tags come with an extra row and these characters at the end.
    // See http://stackoverflow.com/questions/15472107/when-listing-git-ls-remote-why-theres-after-the-tag-name
    if (substr($line_string, -3, 3) == '^{}') {
      continue;
    }

    // Example remote line:
    // 9fc5727c0823d8d3300ba5aae3328d5998033e45 refs/heads/master
    // 9fc5727c0823d8d3300ba5aae3328d5998033e45 refs/tags/1.0
    $line = trim(substr($line_string, 40));
    if (empty($line) || $line == 'HEAD') {
      continue;
    }

    // If branch
    if (strpos($line, 'refs/heads/') === 0) {
      $git_ref = str_replace('refs/heads/', '', $line);
      $branches[] = $git_ref;
      $refs[$git_ref] = 'branch';
    }
    // else if tag
    elseif (strpos($line, 'refs/tags/') === 0) {
      $git_ref = str_replace('refs/tags/', '', $line);
      $tags[] = $git_ref;
      $refs[$git_ref] = 'tag';
    }
    // If not a tag or a head, continue.
    // @TODO: Should we store alternative types? GitHub Pull Requests use this.
    else {
      continue;
    }
  }
  drush_log(dt('Found !count branches: !list', array(
    '!count' => count($branches),
    '!list' => implode(', ', $branches),
  )), 'p_log');
  drush_log(dt('Found !count tags: !list', array(
    '!count' => count($tags),
    '!list' => implode(', ', $tags),
  )), 'p_log');

  return array('branches' => $branches, 'tags' => $tags, 'refs' => $refs);
}
