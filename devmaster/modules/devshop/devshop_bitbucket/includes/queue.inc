<?php
/**
 * @file queue.inc
 * Contains hooks to create a Hosting Queue...
 */

/**
 * Implements hook_hosting_queues().
 */
function devshop_bitbucket_hosting_queues() {
  $items['bitbucket'] = array(
    'type' => 'batch',
    'name' => t('BitBucket Pull Requests'),
    'description' => t('BitBucket Polling: Keep environments up to date with latest code and maintain PR environments.'),
    'total_items' => 1,
    'frequency' => strtotime("1 minute", 0),
    'singular' => t('project'),
    'plural' => t('projects'),
  );
  return $items;
}

/**
 * Implements hosting_QUEUE_TYPE_queue().
 *
 * Provides the `hosting-bitbucket` drush command.
 *
 * This is run by cron, via the hosting queues system.
 */
function hosting_bitbucket_queue($count) {

  // Load all published projects.
  $query = db_select('hosting_devshop_project', 'p');
  $query->join('node', 'n', 'p.nid = n.nid');
  $query
    ->fields('p', array('nid'))
    ->condition('n.status', NODE_PUBLISHED)
    ->execute();

  $projects = $query->execute()->fetchAll();

  // Look through all projects...
  foreach ($projects as $project) {
    $project_node = node_load($project->nid);
    $project = $project_node->project;

    drush_log("Checking project {$project->name}...", "success");

    // Check ones that have PR envs enabled
    if (!empty($project->settings->bitbucket['pull_request_environments'])) {
      drush_log("Project {$project->name} is configured to maintain environments for all pull requests. Preparing environments...", "success");
      $message = devshop_bitbucket_create_all_pr_environments($project_node);
      drush_log($message, "success");
    }
  }
//
//  print "Queue done: \n";
//  return TRUE;
}


/**
 * Function to create all PR environments for a project.
 */
function devshop_bitbucket_create_all_pr_environments($project_node) {

  // Load all PRs from GitHub API
  $project = $project_node->project;
  try {
    $list = devshop_bitbucket_client()->repositories()
        ->workspaces($project->bitbucket_owner)
        ->pullRequests($project->bitbucket_repo)
        ->list()
    ;
    
    $prs_data = $list['values'];
  } catch (\Exception $e) {
    drupal_set_message(t("Unable to load Pull Requests: !message", array('!message' => $e->getMessage())));
    return;
  }
  $prs = array();
  foreach ($prs_data as $pr) {
    $pr = convert_to_object($pr);
    $slug = "pr{$pr->id}";
    $prs[$slug] = $pr;
  }
  $prs_to_create_environments_for = $prs;
  $deployed_environments = 0;
  $deleted_environments = 0;

  // Step 1: Update existing existing environments, populate $prs_to_create_environments_for,
  // and queue "delete", "deploy", or "install" tasks for PRs depending on their state.
  // For each existing PR environment on DevShop:
  foreach ($project->environments as $environment) {
    $site_url = url("node/{$environment->site}", array('absolute' => true));

    // Get PR ID, if it already has one.
    if (!empty($environment->bitbucket_pull_request->id)) {
      $environment_pr_number = $environment->bitbucket_pull_request->id;
      $environment_pr_slug = "pr{$environment_pr_number}";
    }
    else {
      $environment_pr_number = NULL;
      $environment_pr_slug = NULL;
    }

    drush_log("Checking $environment->name... [$site_url]", 'ok');

    // If no PR data in environment and no PR exists with this environments name, it is not a PR environment. Move on.
    if (empty($prs[$environment->name]) && $environment->site_status != HOSTING_SITE_DELETED && empty($environment->bitbucket_pull_request)) {
      continue;
    }
    // If no PR data, but the environment name matches a PR's slug, update data.
    // Handles the edge case where a pr123 environment lost it's github PR info.
    elseif (!$environment_pr_number && !empty($prs[$environment->name])) {
      devshop_bitbucket_save_pr_env_data($prs[$environment->name], $environment);
      drush_log(
        "Environment found with the name '$environment->name'. Assuming it is a PR environment. Updating it with PR data and removing it from the list of environments to create. [$site_url]",
        "warning"
      );
      unset($prs_to_create_environments_for[$environment->name]);
    }
    // If there IS PR data in the environment and the PR exists on GitHub, update the PR data.
    elseif ($environment_pr_number && !empty($prs[$environment_pr_slug]) && !empty($environment->tasks['delete'])) {
      drush_log("Deleted PR environment found: $environment->name [$site_url]", "ok");

      // Save data
      devshop_bitbucket_save_pr_env_data($prs[$environment_pr_slug], $environment);
    }
    // If there IS PR data in the environment and the PR exists on bitbucket, update the PR data.
    elseif ($environment_pr_number && !empty($prs[$environment_pr_slug])) {
      devshop_bitbucket_save_pr_env_data($prs[$environment_pr_slug], $environment);
      drush_log("PR environment found and updated: $environment->name [$site_url]", "ok");

      // If this environment is a "automated" PR environment, unset it from the list of envs to create.
      if ($environment_pr_slug == $environment->name) {
        unset($prs_to_create_environments_for[$environment_pr_slug]);
      }
    }
    // If there IS PR data stored on this environment but the PR does NOT exist, 
    elseif ($environment_pr_number && empty($prs[$environment_pr_slug]) && $environment->site_status == HOSTING_SITE_DELETED) {
      drush_log("PR environment already deleted: $environment->name [$site_url]", "ok");
    }

    // If there IS PR data stored on this environment but the PR does NOT exist, and the site isn't already deleted, queue delete task.
    elseif ($environment_pr_number && empty($prs[$environment_pr_slug]) && $environment->site_status != HOSTING_SITE_DELETED) {
      drush_log("PR $environment_pr_number environment $environment->name exists but PR has been closed. [$site_url] [PR Slug $environment_pr_slug]", "ok");

      $delete_tasks_running = isset($environment->tasks['delete']) && array_filter($environment->tasks['delete'], function($value) {
          return $value->task_status == HOSTING_TASK_QUEUED || $value->task_status == HOSTING_TASK_PROCESSING;
        });

      if ($project->settings->bitbucket['pull_request_environments_delete'] && empty($delete_tasks_running)) {
        drush_log("PR environment scheduled for deletion: $environment->name [$site_url]","ok");
        hosting_add_task($environment->site, 'delete', array('force' => 1));
        $deleted_environments++;
      }
      elseif ($delete_tasks_running) {
        drush_log("PR environment already scheduled for deletion: $environment->name [$site_url]", "ok");
      }

      // Since this PR is closed, and the site is scheduled for deletion, don't
      // run any further tasks on the site.
      continue;
    }
//
//    // Deploy Task Detection.
//    // Read current SHA from the environment repo_path
//    $environment->git_sha = file_exists($environment->repo_path)? trim(shell_exec("cd {$environment->repo_path}; git rev-parse HEAD  2> /dev/null")): NULL;
//
//    // If Site was installed properly and is still enabled, and repo_path is a real folder, and environment git_sha is not empty, and local git_sha does not match github PR sha...
//    if (isset($prs[$environment_pr_slug])) {
//      $pr_object = $prs[$environment_pr_slug];
//      if (file_exists($environment->repo_path) && !empty($environment->git_sha) &&  $environment->git_sha != $pr_object->head->sha) {
//        drush_log(dt("Environment @name SHA (@e) and PR SHA (@p) are different. Preparing a deploy task... [@url] ", array(
//          '@name' => $environment->name,
//          '@e' => $environment->git_sha,
//          '@p' => $pr_object->head->sha,
//          '@url' => $site_url,
//        )), "ok");
//
//
//        // Check for queued deploy tasks. If there are none, create one.
//        $queued_deploy_tasks = hosting_get_new_tasks(1, $environment->site, 'deploy');
//        if (count($queued_deploy_tasks) == 0) {
//
//          // Trigger deploy task to get latest code.
//          // Pass no args to skip DB updates, cache clearing, etc.
//          drush_log("PR environment scheduled for deployment: $environment->name $environment->git_ref [$site_url]", "ok");
//          hosting_add_task($environment->site, 'deploy', array(
//            'git_ref' => $environment->git_ref,
//          ));
//          $deployed_environments++;
//        }
//        else {
//          drush_log("PR environment deployment skipped: $environment->name $environment->git_ref [$site_url]", "ok");
//        }
//
//        // If site is set to rebuild, trigger "install" with force-reinstall
//        if ($project->settings->github['pull_request_reinstall']) {
//          drush_log("PR environment scheduled for reinstallation: $environment->name [$site_url]", "ok");
//          $queued_install_tasks = hosting_get_new_tasks(1, $environment->site, 'install');
//          if (count($queued_install_tasks) == 0) {
//            hosting_add_task($environment->site, 'install', array('force-reinstall' => 1));
//          }
//          else {
//            drush_log("PR install task skipped: there is already a task in the queue.", "ok");
//          }
//        }
//      }
//    }
    drush_log("PR Processing complete for $environment->name", "ok");
    drush_log("", "ok");
    drush_log("", "ok");
    drush_log("", "ok");

  }

  // If pull_request_environments_method method set to "_primary", and live_environment is not set,throw an error.
  if ($project->settings->bitbucket['pull_request_environments_method'] == '_primary' && empty($project->settings->live['live_environment'])) {
    return drush_set_error("DRUSH_FRAMEWORK_ERROR", 'PR environments set to clone "Primary Environment", but there is none configured. Visit Project Settings > General Settings and choose a Primary Environment.');
  }

  //  If pull_request_environments_method method set to "_primary", reset 'pull_request_environments_method' to the name of the primary environment.
  elseif ($project->settings->bitbucket['pull_request_environments_method'] == '_primary' && !empty($project->settings->live['live_environment'])) {
    $primary_env_name = $project->settings->live['live_environment'];
    $project->settings->bitbucket['pull_request_environments_method'] = $primary_env_name;
  }

  // Step 2: For all PRS that do not have an environment, create one.
  foreach ($prs_to_create_environments_for as $slug => $pr) {
    drush_log("Creating environment for PR $pr->number ...", "ok");
    $branch = $pr->head->ref;

    // If PR has different remote URL, set it.
    if ($pr->head->repo->fork) {
      // Try to match project URL scheme
      if (strpos($project->git_url, 'git://') === 0){
        $remote_url = $pr->head->repo->git_url;
      }
      elseif (strpos($project->git_url, 'git@') === 0){
        $remote_url = $pr->head->repo->ssh_url;
      }
      else {
        $remote_url = $pr->head->repo->clone_url;
      }
    }

    // If not a fork, use the same git_ref as project.
    else {
      $remote_url = $project->git_ref;
    }

    // @TODO: Create a "PR Env pattern" setting.
    $environment_name = $slug;

    // If method is "install"...
    if ($project->settings->bitbucket['pull_request_environments_method'] == 'devshop__bitbucket__install') {

      // Create environment then save PR data.
      // @TODO: Unify this into one function.
      $environment = hosting_create_environment($project, $environment_name, $branch, NULL, NULL, 'create', $remote_url);
      //      DevShopGitHubApi::createDeployment($environment, 'queued');
      devshop_bitbucket_save_pr_env_data($pr, $environment);

      drush_log("Environment $environment_name created for $project_node->title via installation profile.", "ok");
    }
    // If environment exists for "pull_request_environments_method" setting, clone it.
    elseif (isset($project->environments[$project->settings->bitbucket['pull_request_environments_method']])) {
      $source_env = $project->settings->bitbucket['pull_request_environments_method'];

      // Create environment then save PR data.
      // @TODO: Unify this into one function.
      $environment = hosting_create_environment($project, $environment_name, $branch, $source_env, NULL, 'create', $remote_url);

      //      DevShopGitHubApi::createDeployment($environment, 'queued');
      devshop_bitbucket_save_pr_env_data($pr, $environment);
      drush_log("Environment $environment_name created for $project_node->title via cloning $source_env...", "ok");

      // Queue up a "deploy" task so that the same hooks are run post-clone.
      // @TODO: Decide if a setting is needed to control this behavior.
      // @TODO: Reduce duplication with the github webhook.
      // @TODO: Make this a global setting for all "clone" environment installs.
      drush_log("PR environment scheduled for deployment: $environment->name $environment->git_ref [$site_url]", "ok");
      hosting_add_task($environment->site, 'deploy', array(
        'git_ref' => $branch,
      ));

    }
    // If environment does not exist, note that error.
    elseif (!isset($project->environments[$project->settings->bitbucket['pull_request_environments_method']])) {
      drush_log("No environment found named {$project->settings->bitbucket['pull_request_environments_method']}", "warning");
    }
    // If "pull_request_environments_method" is something else, we don't know what to do.
    else {
      drush_log("Unknown PR environment creation method: {$project->settings->bitbucket['pull_request_environments_method']}", "warning");
    }
  }

  // Process $prs_to_create_environments_for
  drush_log( format_plural(count($prs_to_create_environments_for),  "1 environment created.", t("!count Environments created.", array(
    "!count" => count($prs_to_create_environments_for),
  ))), "ok");

  drush_log( format_plural($deployed_environments,  "1 Environment Updated.", t("!count Environments Updated.", array(
    "!count" => $deployed_environments,
  ))), "ok");

  drush_log( format_plural($deleted_environments,  "1 Environment Deleted.", t("!count Environments Deleted.", array(
    "!count" => $deleted_environments,
  ))), "ok");
}