<?php
/**
 * @file queue.inc
 * Contains hooks to create a Hosting Queue...
 */

/**
 * Implements hook_hosting_queues().
 */
function devshop_github_hosting_queues() {
  $items['github'] = array(
    'type' => 'batch',
    'name' => t('GitHub'),
    'description' => t('GitHub Polling: Keep environments up to date with latest code and maintain PR environments.'),
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
 * Provides the `hosting-github` drush command.
 *
 * This is run by cron, via the hosting queues system.
 */
function hosting_github_queue($count) {

  // Load all published projects.
  $query = db_select('hosting_devshop_project', 'p');
  $query->join('node', 'n', 'p.nid = n.nid');
  $query
    ->fields('p', array('nid'))
    ->condition('n.status', NODE_PUBLISHED)
    ->execute();

  $projects = $query->execute()->fetchAll();

  // Load GitHub API
  // @TODO: Move to devshop_github_client() ?
  try {
    $pr_api = devshop_github_client()->pullRequests();
  } catch (\Exception $e) {
    watchdog('hosting_queue', 'GitHub API Client Failed: ' . $e->getMessage());
    return;
  }

  // Look through all projects...
  foreach ($projects as $project) {
    $project_node = node_load($project->nid);
    $project = $project_node->project;

    print "Checking project {$project->name}... \n";

    // Check ones that have PR envs enabled
    if ($project->settings->github['pull_request_environments']) {
      print "PR envs enabled for {$project->name}... \n";

      // Load all PRs from GitHub API
      $prs = $pr_api->all($project->github_owner, $project->github_repo);

      foreach ($prs as &$pr) {
        $pr = (object) $pr;
        $prs_with_no_environment["pr{$pr->number}"] = $pr;
      }

      // For each existing environment...
      foreach ($project->environments as $devshop_environment_name => $environment) {
        print "Checking $devshop_environment_name... \n";

        // If PR data exists, and git SHA does not match PR sha: create deploy task.
        if (isset($environment->github_pull_request)) {
          print "@TODO: Update existing PR environments data! \n";
          print "@TODO: Remove from $prs_with_no_environment ! \n";
          print "Environment exists. Removing from list of that need to be created. \n";

          devshop_github_save_pr_env_data($prs_with_no_environment[$devshop_environment_name], $environment);
          unset($prs_with_no_environment[$devshop_environment_name]);
        }
        else {
          devshop_github_save_pr_env_data($prs_with_no_environment[$devshop_environment_name], $environment);
          print "Environment exists with the name $devshop_environment_name but No PR data saved for this environment, so it was updated. \n";
        }
      }

      // For all PRS that do not have an environment, create one.
      foreach ($prs_with_no_environment as $pr_data) {
        $pr = (object) $pr_data;
        $message = "Creating environment for PR $pr->number ... \n";
        $branch = $pr->head['ref'];

        // @TODO: Create a "PR Env pattern" setting.
        $environment_name = "pr" . $pr->number;

        // If method is "install"...
        if ($project->settings->github['pull_request_environments_method'] == 'devshop__github__install') {
          hosting_create_environment($project, $environment_name, $branch);
          $message .= "Environment $environment_name created for $project_node->title via installation profile.\n";
        }
        // If environment exists for "pull_request_environments_method" setting, clone it.
        elseif (isset($project->environments[$project->settings->github['pull_request_environments_method']])) {
          $source_env = $project->settings->github['pull_request_environments_method'];
          hosting_create_environment($project, $environment_name, $branch, $source_env);
          $message .= "Environment $environment_name created for $project_node->title via cloning $source_env \n";
        }
        // If environment does not exist, note that error.
        elseif (!isset($project->environments[$project->settings->github['pull_request_environments_method']])) {
          $message .= "No environment found named {$project->settings->github['pull_request_environments_method']} \n";
        }
        // If "pull_request_environments_method" is something else, we don't know what to do.
        else {
          $message .= "Unknown PR environment creation method: {$project->settings->github['pull_request_environments_method']} \n";
        }
      }
    }

    // Reload project data
    $project = node_load($project->nid);

    // Save latest PR object to all environments.
    foreach ($prs as $pr) {
      if (isset($project->environments["pr{$pr->number}"])) {
        devshop_github_save_pr_env_data($pr, $project->environments["pr{$pr->number}"]);
      }
    }
  }

  print "Queue done: \n";
  print $message;
  return TRUE;
}

