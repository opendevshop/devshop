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
    $client = devshop_github_client();
    $pr_api = $client->pullRequests();
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
      $prs_data = $pr_api->all($project->github_owner, $project->github_repo);
      foreach ($prs_data as $pr) {
        $pr = (object) $pr;
        $slug = "pr{$pr->number}";
        $prs[$slug] = $pr;
        $envs_to_create[$slug] = $pr;
      }

      // Start with the full list.
      $envs_to_create = $prs;

      // For each existing environment...
      foreach ($project->environments as $devshop_environment_name => $environment) {
        print "Checking $devshop_environment_name... \n";

        // If PR data exists, and git SHA does not match PR sha: create deploy task.
        $prs[$devshop_environment_name]->user = (object) $prs[$devshop_environment_name]->user;
        if (!empty($environment->github_pull_request)) {
          print "Environment $devshop_environment_name exists. Saving updated PR data. \n";

          // @TODO: Figure out how to alter response from GitHub API for this request... See ResponseMediator.php in KNPLabs/github-api
          devshop_github_save_pr_env_data($prs[$devshop_environment_name], $environment);
          unset($envs_to_create[$devshop_environment_name]);
        }
        else {
          devshop_github_save_pr_env_data($prs[$devshop_environment_name], $environment);
          print "Environment exists with the name $devshop_environment_name but no PR data existed. Saving new PR data. \n";
          unset($envs_to_create[$devshop_environment_name]);
        }
      }

      // List all environments to be created.
      echo "PR environment discovery complete: \n";
      echo format_plural(count($envs_to_create),  "1 environment to be created: ", t("!count Environments to be created: ", array(
        "!count" => count($envs_to_create),
      )));

      // For all PRS that do not have an environment, create one.
      foreach ($envs_to_create as $slug => $pr) {
        $message = "Creating environment for PR $pr->number ... \n";
        $branch = $pr->head['ref'];

        // @TODO: Create a "PR Env pattern" setting.
        $environment_name = $slug;

        // If method is "install"...
        if ($project->settings->github['pull_request_environments_method'] == 'devshop__github__install') {

          // Create environment then save PR data.
          // @TODO: Unify this into one function.
          $environment = hosting_create_environment($project, $environment_name, $branch);
          devshop_github_save_pr_env_data($pr, $environment);

          $message .= "Environment $environment_name created for $project_node->title via installation profile.\n";
        }
        // If environment exists for "pull_request_environments_method" setting, clone it.
        elseif (isset($project->environments[$project->settings->github['pull_request_environments_method']])) {
          $source_env = $project->settings->github['pull_request_environments_method'];

          // Create environment then save PR data.
          // @TODO: Unify this into one function.
          $environment = hosting_create_environment($project, $environment_name, $branch, $source_env);
          devshop_github_save_pr_env_data($pr, $environment);
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
  }

  print "Queue done: \n";
  print $message;
  return TRUE;
}

