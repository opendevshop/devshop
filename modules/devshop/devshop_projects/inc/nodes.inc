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
    "base" => 'devshop_project',
    "description" => t('A project is a website with multiple environments.'),
    "has_title" => TRUE,
    "title_label" => t('Project Codename'),
    "locked" => TRUE,
  );
  return $types;
}

/**
 * Implements hook_node_load().
 *
 * Loads data into Project nodes.
 */
function devshop_projects_node_load($nodes, $types) {

  // Only act on project and related nodes.
  $types = array(
    'project',
    'site',
    'platform',
    'task',
  );
  if (count(array_intersect($types, $types)) == 0) {
    return;
  }

  global $user;
  $project = new stdClass();

  foreach ($nodes as $nid => &$node) {

    // Tasks
    if ($node->type == 'task') {

      // Prepare bootstrap-compatible things.
      // Load up task types to get our language.
      $task_types = hosting_available_tasks();
      $task_types = array_merge($task_types['site'], $task_types['platform']);

      // In Aegir, "executed" and "delta" does not get updated properly.
      // Handle "delta" being not computed yet
      if ($node->task_status == HOSTING_TASK_PROCESSING) {
        $node->duration = format_interval(time() - $node->executed, 1);
      }
      elseif ($node->task_status == HOSTING_TASK_QUEUED) {
        $node->duration = t('Queued for %time', array('%time' => format_interval(time() - $node->changed)));
      }
      else  {
        $node->duration = format_interval($node->delta, 1);
      }

      // To test out constantly changing tasks, uncomment this line.
      //        $node->task_status = rand(-1, 3);
      $node->status_class = devshop_task_status_class($node->task_status);
      $node->status_name = _hosting_parse_error_code($node->task_status);

      // Override "ago" text.
      $node->ago = format_interval(time() - $node->executed, 1) .' '. t('ago');
      if ($node->task_status == HOSTING_TASK_QUEUED) {
        $node->ago = t('Queued for %time', array('%time' => format_interval(time() - $node->changed)));
      }
      elseif ($node->task_status == HOSTING_TASK_PROCESSING) {
        $node->ago = format_interval(time() - $node->changed);
      }

      // Set task type
      $node->type_name = $task_types[$node->task_type]['title'];

      // If platform's first verify...
      if (isset($node->ref) && $node->ref->type == 'platform' && $node->ref->verified == 0) {
        $node->type_name = t('Cloning Codebase');
      }

      // Get Icon
      $node->icon = devshop_task_status_icon($node->task_status);

      // Get desired URL
      $node->url = url("node/{$node->nid}");

      // @TODO: Doesn't quite work. we need a different VID for each clone link.
      //  if ($node->task_type == 'clone') {
      //    $additions['url'] = url("node/{$node->nid}/revisions/{$node->vid}/view");
      //  }
    }
    // Projects
    elseif ($node->type == 'project') {

      // Load the "hosting context". The unique name in the aegir system.
      $node->project_name = $node->title;
      $node->hosting_name = 'project_' . $node->title;

      $node->project = devshop_project_load($node);
    }
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

    if (($site->type == 'site' || $site->type == 'platform') && isset($site->environment) && ($node->task_type == HOSTING_TASK_QUEUED || $node->task_type == HOSTING_TASK_PROCESSING)) {
      $site->environment->last_task = $node->nid;
      devshop_environment_save_last_task($site->environment);
    }
  }

  if ($node->type != 'project') {
    return;
  }

  if (!isset($node->no_verify)) {
    hosting_add_task($node->nid, 'verify');
  }

  $info = new stdClass();
  $info->nid = $node->nid;
  $info->git_url = $node->project->git_url;
  $info->code_path = hosting_path_normalize($node->project->code_path);
  $info->drupal_path = hosting_path_normalize($node->project->drupal_path);
  $info->base_url = $node->project->base_url;
  $info->install_profile = $node->project->install_profile;

  // Save serialized data, minus environments
  $info->settings = serialize($node->project->settings);

  drupal_write_record('hosting_devshop_project', $info);

  // Save hosting context
  if (!$node->old_vid) {

    // Save the "hosting context", which includes setting the path alias.
    $name = ($node->hosting_name) ? $node->hosting_name : $node->title;
    hosting_context_register($node->nid, $name);

    // Replace the alias created by hosting_context_register.
    $path['source'] = "node/{$node->nid}";
    $path['alias'] = "project/{$name}";
    path_save($path);
  }

  // @TODO: The wizard always creates the project before the environments.
  // Not sure if we need this, but we might to enable importing via drush.
  // Save Environment records.
  if (!empty($node->project->environments)) {
    foreach ($node->project->environments as $name => $environment) {
      // Ensure correct data types
      $environment = (object) $environment;
      $environment->settings = (array) $environment->settings;

      $info = new stdClass();
      $info->project_nid = $node->nid;
      $info->name = $name;
      $info->git_ref = $environment->git_ref;
      $info->site = $environment->site;
      $info->platform = $environment->platform;

      // Remove primary settings from settings array before saving.
      unset($environment->settings['git_ref']);
      unset($environment->settings['site']);
      unset($environment->settings['platform']);
      $info->settings = serialize($environment->settings);

      // Save environment record.
      drupal_write_record('hosting_devshop_project_environment', $info);
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
  $info->install_profile = isset($project->install_profile) ? $project->install_profile : '';

  // Save serialized data, minus environments
  $info->settings = serialize($project->settings);

  // Write project record.
  drupal_write_record('hosting_devshop_project', $info, 'nid');
  if (!isset($node->no_verify) || $node->no_verify == FALSE) {
    hosting_add_task($node->nid, 'verify');
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
}

///**
// * Implementation of hook_nodeapi()
// * For site and platform nodes, load project and environment objects.
// *
// * @see devshop_projects_load()
// */
//function devshop_projects_nodeapi(&$node, $op, $a3 = null) {

//  // Load environment info into platforms and sites.
//  if ($op == 'load' && $node->type == 'platform' || $node->type == 'site') {
//    $additions = array();
//    $result = (object) db_query("SELECT project_nid, name FROM {hosting_devshop_project_environment} e WHERE :type  = :nid", array(
//        ':type' => $node->type,
//        ':nid'  => $node->nid
//    ))->fetchAssoc();
//
//    // Load project and environment
//    $project_node = node_load($result->project_nid);
//    $additions['project'] = $project_node->project;
//    $additions['environment'] = $project_node->project->environments[$result->name];
//    return $additions;
//  }

//  // Load environment info into tasks.
//  if ($op == 'load' && $node->type == 'task') {
//    $additions = array();
//    $result = (object) db_query("SELECT nid, title AS name, name AS environment FROM {hosting_devshop_project_environment} e LEFT JOIN node ON e.project_nid = node.nid WHERE site = :site OR platform = :platform", array(
//        ':site' => $node->rid,
//        ':platform' => $node->rid
//    ))->fetchAssoc();
//
//    // Load project and environment
//    $additions['project'] = $result;
//    return $additions;
//  }
//
//  // When a task is updated, save the last_task status in the environment settings.
//  if (($op == 'insert' || $op == 'update') && $node->type == 'task') {
//    $site = node_load($node->rid);
//
//    if (($site->type == 'site' || $site->type == 'platform') && isset($site->environment) && ($node->task_type == HOSTING_TASK_QUEUED || $node->task_type == HOSTING_TASK_PROCESSING)) {
//
//      $site->environment->last_task = $node->nid;
//      devshop_environment_save_last_task($site->environment);
//    }
//  }
//
//  // When a task is created or updated that has a project, redirect to the project.
//  if (($op == 'update' || $op == 'insert') && $node->type == 'task') {
//    $object = node_load($node->rid);
//    if (isset($object->project) && arg(0) == 'node' && arg(1) == $object->nid) {
//      drupal_goto("node/{$object->project->nid}");
//
//    }
//  }
//
//  // When a platform or site node is deleted, remove the environment record.
//  if ($op == 'delete' && ($node->type == 'site' || $node->type == 'platform')) {
//    drupal_set_message('Deleted environment.');
//    db_delete('hosting_devshop_project_environment')
//      ->condition($node->type, $node->nid)
//      ->execute();
//  }
//}

///**
// * Implements hook_nodeapi_TYPE_OP() for site nodes and view op.
// */
//function devshop_projects_nodeapi_task_load(&$node, $a3, $a4) {
//
//  $task_types = hosting_available_tasks();
//  $task_types = array_merge($task_types['site'], $task_types['platform']);
//
//  // In Aegir, "executed" and "delta" does not get updated properly.
//  // Handle "delta" being not computed yet
//  if ($node->task_status == HOSTING_TASK_PROCESSING) {
//    $additions['duration'] = format_interval(time() - $node->executed, 1);
//  }
//  elseif ($node->task_status == HOSTING_TASK_QUEUED) {
//    $additions['duration'] = t('Queued for %time', array('%time' => format_interval(time() - $node->changed)));
//  }
//  else  {
//    $additions['duration'] = format_interval($node->delta, 1);
//  }
//
//  // To test out constantly changing tasks, uncomment this line.
//  //        $node->task_status = rand(-1, 3);
//  $additions['status_class'] = devshop_task_status_class($node->task_status);
//  $additions['status_name'] = _hosting_parse_error_code($node->task_status);
//
//  // Override "ago" text.
//  $additions['ago'] = format_interval(time() - $node->executed, 1) .' '. t('ago');
//  if ($node->task_status == HOSTING_TASK_QUEUED) {
//    $additions['ago'] = t('Queued for %time', array('%time' => format_interval(time() - $node->changed)));
//  }
//  elseif ($node->task_status == HOSTING_TASK_PROCESSING) {
//    $additions['ago'] = format_interval(time() - $node->changed);
//
//  }
//
//  // Set task type
//  $additions['type_name'] = $task_types[$node->task_type]['title'];
//
//  // If platform's first verify...
//  if ($node->ref->type == 'platform' && $node->ref->verified == 0) {
//    $additions['type_name'] = t('Cloning Codebase');
//  }
//
//  // Get Icon
//  $additions['icon'] = devshop_task_status_icon($node->task_status);
//
//  // Get desired URL
//  $additions['url'] = url("node/{$node->nid}");
//
//  // @TODO: Doesn't quite work. we need a different VID for each clone link.
//  //  if ($node->task_type == 'clone') {
//  //    $additions['url'] = url("node/{$node->nid}/revisions/{$node->vid}/view");
//  //  }
//  return $additions;
//}

/**
 * @param $environment
 */
function devshop_environment_save(stdClass &$environment) {
  $environment->settings = (array) $environment->settings;

  // Prepare record for saving
  $info = new stdClass();
  $info->project_nid = $environment->project_nid;
  $info->name = $environment->name;
  $info->git_ref = $environment->git_ref;
  $info->site = $environment->site;
  $info->platform = $environment->platform;
  $info->settings = serialize($environment->settings);
  $info->last_task = $environment->last_task;

  // Save environment record.
  if (drupal_write_record('hosting_devshop_project_environment', $info, array('project_nid', 'name'))) {
    watchdog('ok', 'Environment record saved: ' . print_r($info, 1));
    return TRUE;
  }
  else {
    watchdog('error', 'Environment record not saved: ' . print_r($info, 1));
    return FALSE;
  }
}

/**
 * Helper to save last task info.
 */
function devshop_environment_save_last_task($environment) {

  db_update('hosting_devshop_project_environment')
    ->fields(array(
      'last_task' => $environment->last_task,
    ))
    ->condition('name', $environment->name)
    ->condition('project_nid', $environment->project_nid)
    ->execute();
}
