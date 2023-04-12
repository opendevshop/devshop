<?php
/**
 * @file devshop_project.node.inc
 *
 * DevShop Project Node related hooks and support functions.
 *
 */

/**
 * Implementation of hook_node_info().
 */
function devshop_projects_node_info() {
  $types["project"] =  array(
    "name" => 'DevShop Project',
    "base" => 'devshop_projects',
    "description" => t('A project is a website with multiple environments.'),
    "has_title" => TRUE,
    "title_label" => t('Project Codename'),
    "locked" => TRUE,
  );
  return $types;
}

function devshop_projects_node_prepare($node) {
  
  // Load up project hosting settings on create site page.
  if ($node->type == 'site' && empty($node->hosting_settings) && arg(1) == 'add' && !empty(arg(3))) {
    $project = devshop_projects_load_by_name(arg(3));
    $node->hosting_settings = $project->hosting_settings;
  }
}

/**
 * Implements hook_node_load().
 *
 * Loads data into Project nodes.
 */
function devshop_projects_node_load($nodes, $types) {

  // Only act on project and related nodes.
  $allowed_types = array(
    'project',
    'site',
    'platform',
    'task',
  );
  if (count(array_intersect($allowed_types, $types)) == 0) {
    return;
  }

  $refs_types = db_select('node', 'n')
    ->fields('n', array('nid', 'type'))
    ->condition('type', array('site', 'platform', 'project', 'server'), 'IN')
    ->execute()
    ->fetchAllKeyed();

  foreach ($nodes as $nid => &$node) {

    // Tasks
    if ($node->type == 'task') {

      // Prepare bootstrap-compatible things.
      // Load up task types to get our language.
      $task_types = hosting_available_tasks();

      // In Aegir, "executed" and "delta" does not get updated properly.
      // Handle "delta" being not computed yet
      if ($node->task_status == HOSTING_TASK_PROCESSING) {
        $node->duration = format_interval(time() - $node->executed, 1);
      }
      elseif ($node->task_status == HOSTING_TASK_QUEUED) {
        $node->duration = t('Queued for %time', array('%time' => format_interval(time() - $node->changed)));
      }
      else {
        $node->duration = format_interval($node->delta, 1);
      }

      // To test out constantly changing tasks, uncomment this line.
      //        $node->task_status = rand(-1, 3);
      $node->status_class = devshop_task_status_class($node->task_status);
      $node->status_bootstrap_class = devshop_task_status_bootstrap_class($node->task_status);
      $node->status_name = _hosting_parse_error_code($node->task_status);
      $node->status_slug = strtolower($node->status_name);

      // Override "ago" text.
      $node->ago = format_interval(time() - $node->executed,
          1) . ' ' . t('ago');
      if ($node->task_status == HOSTING_TASK_QUEUED) {
        $node->ago = t('Queued for %time',
          array('%time' => format_interval(time() - $node->changed)));
      }
      elseif ($node->task_status == HOSTING_TASK_PROCESSING) {
        $node->ago = format_interval(time() - $node->changed);
      }

      // ISO 8601
      if ($node->executed == 0) {
        $executed = $node->created;
      }
      else {
        $executed = $node->executed;
      }
      $node->task_timestamp = date('c', $executed);
      $node->task_date = format_date($executed);

      // Set task type
      $node->type_name = !empty($task_types[$refs_types[$node->rid]][$node->task_type])? $task_types[$refs_types[$node->rid]][$node->task_type]['title']: $node->task_type;

      // If platform's first verify...
      if (isset($node->ref) && $node->ref->type == 'platform' && $node->ref->verified == 0) {
        $node->type_name = t('Cloning Codebase');
      }

      // Get Icon
      $node->icon = devshop_task_status_icon($node->task_status);

      // Get desired URL
      $node->url = url("node/{$node->nid}");

    }
    // Projects
    elseif ($node->type == 'project') {

      // Load the "hosting context". The unique name in the aegir system.
      $node->project_name = $node->title;
      $node->hosting_name = 'project_' . $node->title;

      $node->project = devshop_project_load($node);
    }
    elseif ($node->type == 'site') {
      // For sites without hosting_settings, load from project. 
      if (empty($node->hosting_settings) && !empty($node->project->hosting_settings)) {
        $node->hosting_settings = $node->project->hosting_settings;
      }
    }
  }
}

/**
 * When install tasks are created, add "force-reinstall" task argument.
 *
 * @param $node
 */
function devshop_projects_node_presave($node) {
  if ($node->type == 'task' && $node->task_type == 'install' && $node->is_new) {
    $node->task_args['force-reinstall'] = 1;
  }
}

/**
 * Implementation of hook_insert().
 *
 *   1. Saves data into our table.
 *   2. Saves a hosting context name.
 *   3. Adds a "Verify" task for this project.
 *
 *  @see hosting_platform_insert()
 */
function devshop_projects_node_insert($node) {

  // On task insert, save last task.
  if ($node->type == 'task') {
    $site = node_load($node->rid);

    // When a task is created or updated that has a project, redirect to the project.
    if (isset($_GET['redirect']) && $_GET['redirect'] == 'task-page') {
      drupal_goto("node/{$node->nid}");
    }
    elseif (isset($site->project) && !empty($site->project->nid) && arg(0) == 'hosting_confirm' && arg(1) == $site->nid) {
      drupal_goto("node/{$site->project->nid}");
    }
  }
// @TODO: Fix this.
//  // On site insert, inherit hosting_settings from the project.
//  if ($node->type == 'site' && empty($node->hosting_settings) && !empty($node->project->hosting_settings)) {
//    $node->hosting_settings = $node->project->hosting_settings;
//  }
    
  if ($node->type != 'project') {
    return;
  }

  if (empty($node->no_verify)) {
    hosting_add_task($node->nid, 'verify');
  }

  // @TODO: Move all to hosting_settings.
  $info = new stdClass();
  $info->nid = $node->nid;
  $info->git_url = $node->project->git_url;
  $info->code_path = hosting_path_normalize($node->project->code_path);
  $info->drupal_path = hosting_path_normalize($node->project->drupal_path);
  $info->base_url = $node->project->base_url;
  $info->install_profile = $node->project->settings->install_method['install_profile'] ?? '';

  // Save serialized data, minus environments
  $info->settings = serialize($node->project->settings);

  drupal_write_record('hosting_devshop_project', $info);

  // Save hosting context
  if (!isset($node->old_vid) || empty($node->old_vid)) {

    // Save the "hosting context", which includes setting the path alias.
    $name = !empty($node->hosting_name) ? $node->hosting_name : $node->title;

    // Ensure "project_" prefix on hosting context name.
    if (strpos($name, 'project_') !== 0) {
      $context_name = 'project_' . $name;
    }
    else {
      $context_name = $name;
    }

    hosting_context_register($node->nid, $context_name);

    // Replace the alias created by hosting_context_register.
    $path['source'] = "node/{$node->nid}";
    $path['alias'] = "project/{$name}";
    path_save($path);
  }

  // Save Environment records, if present.
  // If using Start a Project wizard, environments is NOT set on project node insert.
  // If using drush hosting-import, environments MIGHT be in the data.
  // Soon CLI and API commands will exist to create all of them at once.
  if (!empty($node->project->environments)) {
    $node->project->nid = $node->nid;
    $node->project->name = $node->title;
    foreach ($node->project->environments as $name => $environment) {
      // If site node does not exist, create it.
      if (empty($environment->site)) {
        $environment = devshop_environment_normalize($environment);
        hosting_create_environment($node->project, $name, $environment->git_ref, NULL, $environment->settings, 'create', $environment->git_url);
      }
    }
  }
}

/**
 * Implementation of hook_update().
 *
 *  1. Updates our table.
 *  2. Adds a "Verify" task for this project.
 *
 */
function devshop_projects_node_update($node) {


  // On task insert, save last task.
  if ($node->type == 'task') {
    $site = node_load($node->rid);

    // When a task is created or updated that has a project, redirect to the project.
    if (isset($site->project) && arg(0) == 'hosting_confirm' && arg(1) == $site->nid) {
      drupal_goto("node/{$site->project->nid}");
    }
  }

  if ($node->type != 'project') {
    return;
  }

  $project = (object) $node->project;
  $project->settings = (object) $project->settings;

  $info = new stdClass();
  $info->nid = $node->nid;
  $info->git_url = $project->git_url;
  $info->code_path = hosting_path_normalize($project->code_path);
  $info->drupal_path = hosting_path_normalize($project->drupal_path);
  $info->base_url = $project->base_url;
  $info->install_profile = $project->settings->install_method['install_profile'];

  // Save serialized data, minus environments
  $info->settings = serialize($project->settings);

  // Write project record.
  drupal_write_record('hosting_devshop_project', $info, 'nid');
  if (empty($node->no_verify)) {
    hosting_add_task($node->nid, 'verify');
  }
  
  hosting_context_register($node->nid, $node->hosting_name, $node->hosting_settings);

  $project_node = node_load($node->nid, NULL, TRUE);

  // Environment data (without sites) are present during `hosting-import` drush calls.
  if (!empty($node->project->environments)) {
    $node->project->nid = $node->nid;
    $node->project->name = $node->title;
    foreach ($node->project->environments as $name => $environment) {

      // If site node does not exist, create it.
      $environment = devshop_environment_normalize($environment);
      if (empty($project_node->project->environments[$name]->site)) {
        hosting_create_environment($node->project, $name, $environment->git_ref, NULL, $environment->settings, 'create', $environment->git_url);
      }
      // If site does exist, save the environment data.
      else {
        $environment->project_nid = $node->nid;
        devshop_environment_save($environment);
      }
    }
  }
}

/**
 * A hack until we have actual OOP.
 *
 * @param $environment
 */
function devshop_environment_normalize($environment) {
  // Make sure environments are objects and settings are arrays
  // Preserve settings, convert the rest to all objects.
  if (is_array($environment)) {
   $settings = $environment['settings'];
  }
  elseif (is_object($environment)) {
    $settings = $environment->settings;
  }
  else {
    throw new \Exception(t('devshop_environment_normalize() function requires an object or array'));
  }

  $environment = convert_to_object($environment);
  $environment->settings = (array) $settings;
  return $environment;
}

/**
 * Implements hook_node_delete().
 */
function devshop_projects_node_delete($node) {

  // When a platform or site node is FULLY deleted, remove the environment record.
  // This should never happen, site and platform nodes are never fully deleted.
  if (($node->type == 'site') && !empty($node->project)) {
    db_delete('hosting_devshop_project_environment')
      ->condition('site', $node->nid)
      ->execute();
  }
}

/**
 * Implementation of hook_delete().
 */
function devshop_projects_delete($node) {

  db_delete('hosting_devshop_project')
    ->condition('nid', $node->nid)
    ->execute();

  db_delete('hosting_devshop_project_environment')
  ->condition('project_nid', $node->nid)
  ->execute();

  hosting_context_delete($node->nid);
  hosting_task_delete_related_tasks($node->nid);
}

/**
 * @param $environment
 */
function devshop_environment_save(stdClass &$environment) {
  $environment->settings = (array) $environment->settings;

  // Prepare record for saving
  $info = new stdClass();
  $info->project_nid = $environment->project_nid;
  $info->name = $environment->name;
  $info->site = $environment->site;
  $info->settings = serialize($environment->settings);

  // Check for existing records
  $result = db_select('hosting_devshop_project_environment', 'e')
    ->fields('e', array('project_nid'))
    ->condition('e.name', $info->name)
    ->condition('e.project_nid', $info->project_nid)
    ->execute()
    ->fetchField()
  ;

  $keys = $result? array('site'): array();

  // Save environment record.
  if (drupal_write_record('hosting_devshop_project_environment', $info, $keys)) {
    watchdog('ok', 'Environment record saved: ' . print_r($info, 1));
    return TRUE;
  }
  else {
    watchdog('error', 'Environment record not saved: ' . print_r($info, 1));
    return FALSE;
  }
}

