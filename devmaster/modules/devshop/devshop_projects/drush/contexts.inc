<?php

/**
 * Implements hook_hosting_project_context_options().
 *
 * This transfers data from the node to thes aegir context object (the alias!)
 * For project entities.  This is where we find the branches and tags on the remote.
 */
function devshop_projects_hosting_project_context_options(&$task) {


  $branches = getBranchesAndTags($task->ref->project->git_url);

  $home = variable_get('aegir_home', '/var/aegir');
  $clone_dir = "$home/repos/{$task->ref->project->name}";

  // If something went wrong connecting to the git repo, don't wipe out our branches.
  if (!empty($branches['branches'])) {
    $task->ref->project->settings->git['branches'] = $branches['branches'];
    $task->ref->project->settings->git['tags'] = $branches['tags'];
    $task->ref->project->settings->git['refs'] = $branches['refs'];

    // Save the project node now that we have branches and tags.
    // Don't verify again, this is the verification process.
    // Remove environments, we don't want to update those again.
    $task->ref->no_verify = TRUE;
    $task->ref->environments = NULL;
    node_save($task->ref);
  }

  // If branches are empty, and "github_create" was selected...
  elseif (!file_exists($clone_dir) && $task->ref->project->settings->create_project_settings['git_source'] == 'github_create') {

    $git_url = $task->ref->project->git_url;

    $account = user_load($task->uid);
    $name = $account->name;
    $mail = $account->mail;
    $env = array();

    // Make commit using environment variables for NAME and EMAIL, if specified.
    if (!empty($name)) {
      $env['GIT_AUTHOR_NAME'] = $name;
      $env['GIT_COMMITTER_NAME'] = $name;
    }
    if (!empty($mail)) {
      $env['GIT_AUTHOR_MAIL'] = $mail;
      $env['GIT_COMMITTER_MAIL'] = $mail;
    }

    $hostname = $_SERVER['HOSTNAME'];

    if (!file_exists("$home/repos")) {
      provision_process("mkdir -p $home/repos", null, t('Creating repos directory'));
    }

    if ($task->ref->project->settings->create_project_settings['github_repository_source']['populate_choice'] == 'import') {
      $source_url = $task->ref->project->settings->create_project_settings['github_repository_source']['import'];
      provision_process("git clone $source_url $clone_dir");
      provision_process("git remote set-url origin $git_url", $clone_dir);
    }
    else {

      $composer_project = $task->ref->project->settings->create_project_settings['github_repository_source']['composer_project'];
      $composer_create_project_command = "composer create-project {$composer_project} {$task->ref->project->name} --no-interaction --ansi";

      provision_process($composer_create_project_command,"$home/repos", t('Creating composer project...'));
      $result = drush_get_context('provision_process_result');
      if (!$result->isSuccessful()) {
        return drush_set_error('DEVSHOP_ERROR', 'Composer create-project failed.');
      }

      if (!file_exists($clone_dir)) {
        return drush_set_error('DEVSHOP_ERROR', 'Unable to find codebase in ' . $clone_dir);
      }

      provision_process("git init", $clone_dir);

      provision_process("git add .gitignore", $clone_dir);
      provision_process("git add -A", $clone_dir);
      provision_process("git status", $clone_dir);
      provision_process("git commit -m 'Repo created by DevShop on $hostname using the command `$composer_create_project_command`'", $clone_dir, t('Committing codebase...'), $env);
      provision_process("git remote add origin $git_url", $clone_dir);
    }

    $branch = trim(str_replace('refs/heads/', '', shell_exec("cd {$clone_dir}; git describe --tags --exact-match 2> /dev/null || git symbolic-ref -q HEAD 2> /dev/null")));

    provision_process("git push -u origin $branch", $clone_dir);

    if (file_exists($clone_dir)) {
      provision_process("rm -rf $clone_dir", null, t('Removing temporary git clone...'));
    }

    $branches = getBranchesAndTags($task->ref->project->git_url);
    $task->ref->project->settings->git['branches'] = $branches['branches'];
    $task->ref->project->settings->git['tags'] = $branches['tags'];
    $task->ref->project->settings->git['refs'] = $branches['refs'];

    // Save the project node now that we have branches and tags.
    // Don't verify again, this is the verification process.
    // Remove environments, we don't want to update those again.
    $task->ref->no_verify = TRUE;
    $task->ref->environments = NULL;
    node_save($task->ref);

  }


  // Save project object to drush alias (aegir context).
  if (isset($task->ref->project)) {
    $task->context_options['server'] = '@server_master';
    $task->context_options['project_name'] = $task->ref->title;

    // Clean up project and environment objects.
    $project = $task->ref->project;
    unset($project->tasks);
    unset($project->verify);
    unset($project->messages);

    foreach ($project->environments as &$environment) {
      unset($environment->tasks);
      unset($environment->tasks_list);
    }

    $task->context_options['project'] = $project;
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
  }
}

/**
 * Implements hook_drush_context_import().
 *
 * This allows project nodes to be created from contexts (aliases)
 */
function devshop_projects_drush_context_import($context, &$node) {
  if ($context->type == 'project') {

    if (empty($node->nid)) {
      $node->title = $context->project_name;
      $node->type = 'project';
    }

    $project = (object) $context->project;
    $node->project = new stdClass();
    $node->project->git_url         =   $project->git_url;
    $node->project->code_path       =   $project->code_path;
    $node->project->drupal_path     =   $project->drupal_path;
    $node->project->base_url        =   $project->base_url;
    $node->project->install_profile =   $project->install_profile;
    $node->project->settings        =   $project->settings;

    if (!is_array($project->environments)){
      drush_log(' no environments array ', 'ok');
      return;
    }

    foreach ($project->environments as $name => $environment) {
      $environment = devshop_environment_normalize($environment);

      if (empty($environment->uri)) {
        $environment->uri = devshop_environment_url($project, $name);
      }
      if (empty($environment->name)) {
        $environment->name = $name;
      }
      if (empty($environment->settings)) {
        $environment->settings = array();
      }

      // Backwards compatibility: *_context properties were added, may be missing.
      if (empty($environment->site_context)) {
        $environment->site_context = $environment->uri;
      }
      if (empty($environment->platform_context)) {
        $environment->platform_context = "platform_{$project->name}_{$environment->name}";
      }

      // Load site and platform nodes.
      $site_node = hosting_context_load($environment->site_context);
      $platform_node = hosting_context_load($environment->platform_context);

      // If both site and platform nodes were found, load them in.
      if ($site_node && $platform_node) {
        drush_log(dt('Site and Platform nodes found. Saving environment data for site "!site_context" (nid:!site) and platform "!platform_context" (nid:!platform)', array(
          '!site' => $site_node->nid,
          '!platform' => $platform_node->nid,
          '!site_context' => $environment->site_context,
          '!platform_context' => $environment->platform_context,
        )), 'success');

        $environment->site = $site_node->nid;
        $environment->platform = $platform_node->nid;
      }

      // If hosting-import is being run a second time and the site exists, update it.
      if (!empty($node->nid) && !empty($environment->site)) {
        $environment->project_nid = $node->nid;
        devshop_environment_save($environment);
        drush_log('Updated environment data for ' . $name, 'ok');
      }
      elseif (!empty($node->nid) && empty($environment->site)) {
        drush_log('Creating new environment for ' . $name, 'ok');
        $environment->project_nid = $node->nid;
        $node->project->environments[$name] = $environment;
      }
      elseif (empty($node->nid)) {
        // Project node hasn't been created yet. Add it here for devshop_projects_node_insert() to handle.
        drush_log('Saving new environment data for ' . $name, 'ok');
        $node->project->environments[$name] = $environment;
      }
    }
  }

  // If importing a site that has environment and project assigned,
  elseif ($context->type == 'site' && !empty($context->environment) && !empty($context->project)) {

    // If site, platform, and project already exist, update environment info from alias.
    $project_nid = hosting_context_nid("project_{$context->project}");
    $platform_nid = hosting_context_nid($context->platform);

    if (!empty($node->nid) && $project_nid && $platform_nid) {

      drush_log(dt('Found existing site environment (!environment), and project properties (!project). Saving environment.', array(
        '!environment' => $context->environment,
        '!project' => $context->project,
      )), 'success');

      $environment_object = new stdClass();
      $environment_object->name = $context->environment;
      $environment_object->site = $node->nid;
      $environment_object->platform = $platform_nid;
      $environment_object->settings = $context->environment_settings;
      $environment_object->project_nid = $project_nid;

      if (devshop_environment_save($environment_object)) {
        drush_log(dt('Environment saved.'), 'success');
      }
      else {
        drush_set_error('DEVSHOP_ENVIRONMENT_NOT_SAVED', dt('Environment not saved.'));
      }
    }

    // Site node doesn't exist, will be created after this function.
    elseif (empty($node->nid)) {

      // If project node not found, try to load the context and import that.
      if (empty($project_nid) && $context = d("project_{$context->project}") && $context->type == 'project') {
        drush_log(dt('Project node not found, importing context "!context".', array('!context' => "project_{$context->project}")), 'warning');
        hosting_drush_import($context);

      }
      // If there is a project nid, apply it as a property of the node so the insert hook can handle it.
      elseif (!empty($project_nid)) {
        $node->project_nid = $project_nid;
      }
      // If there is no project nid, and no project context, throw a warning.
      else {
        drush_log(dt('Site property "project" is set to "!project", but there is no project node or context by that name.'), 'warning');
      }
    }
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
