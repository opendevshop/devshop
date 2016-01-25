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

    // Sites & Platforms
    if ($node->type == 'platform' || $node->type == 'site') {

      // Load project and environment
      $result = (object) db_query("SELECT project_nid, name FROM {hosting_devshop_project_environment} e WHERE :type  = :type", array(
          ':type' => $node->type,
      ))->fetchAssoc();

      $project_node = node_load($result->project_nid);

      if (!empty($project_node->project)) {
        $node->project = $project_node->project;
        $node->environment = $project_node->project->environments[$result->name];
      }
      return;
    }

    // Tasks
    elseif ($node->type == 'task') {
      $result = (object) db_query("SELECT nid, name FROM {hosting_devshop_project_environment} e LEFT JOIN node ON e.project_nid = node.nid WHERE site = :site OR platform = :platform", array(
          ':site' => $node->rid,
          ':platform' => $node->rid
      ))->fetchAssoc();

      // Load project and environment
      $project_node = node_load($result->nid);
      $node->project = $project_node->project;
      $node->environment = $project_node->project->environments[$result->name];
    }
    // Projects
    elseif ($node->type == 'project') {

      // Load the "hosting context". The unique name in the aegir system.
      $name = db_query("SELECT name AS hosting_name FROM {hosting_context} WHERE nid = :nid", array(':nid' => $node->nid))->fetchField();
      $node->hosting_name = 'project_' . $name;

      // Load project data
      $project = (object) db_query('SELECT * FROM {hosting_devshop_project} WHERE nid = :nid', array(':nid' => $node->nid))->fetchAssoc();

      // Load up all project settings.
      $project->name = $name;
      $project->settings = (object) unserialize($project->settings);

      // Create "refs" array to determine ref type.
      $project->settings->git['refs'] = array();
      foreach ($project->settings->git as $type => $refs) {
        if (is_array($refs)) {
          foreach ($refs as $ref) {
            $project->settings->git['refs'][$ref] = ($type == 'branches')? 'branch': 'tag';
          }
        }
      }

      // Git Repo Host
      if (strpos($project->git_url, 'github.com') !== FALSE) {
        $project->git_provider = 'github';
        $project->git_repo_url = strtr($project->git_url, array(
          'git@github.com:' => 'http://github.com/',
          'git://' => 'http://',
          '.git' => '',
        ));
      }
      else {
        $project->git_provider = 'git';
        $project->git_repo_url = '';
      }
      // @TODO: Set git providers for most common here, then add a hook to detect.

      // Load Environments
      // @TODO: Remove environments where the site has been deleted.
      $environment_data = db_query("
        SELECT
            e.*,
            s.status as site_status,
            p.status as platform_status,
            p.publish_path as root,
            http.title as remote_host,
            sn.title as uri,
            sn.title as drush_alias,
            http.title as web_server,
            http.nid as web_server_nid,
            db.title as db_server,
            db.nid as db_server_nid,
            n.title as project_name,
            s.vid,
            sn.title as system_domain,
            a.redirection as redirect_domain,
            e.last_task as last_task_nid
          FROM {hosting_devshop_project_environment} e
          LEFT JOIN {hosting_site} s ON e.site = s.nid
          LEFT JOIN {node} sn ON s.vid = sn.vid
          LEFT JOIN {hosting_platform} p ON e.platform = p.nid
          LEFT JOIN {node} http ON p.web_server = http.nid
          LEFT JOIN {node} db ON s.db_server = db.nid
          LEFT JOIN {node} n ON e.project_nid = n.nid
          LEFT JOIN {hosting_site_alias} a ON a.vid = s.vid
          WHERE project_nid = :nid AND
          e.name != ''
          ORDER BY
            name;   ", array(
            ':nid' => $node->nid
      ));
      foreach ($environment_data as $environment) {

        // Don't load the environment if the site and/or platform is deleted.
        if ($environment->site_status == HOSTING_SITE_DELETED && $environment->platform_status == HOSTING_PLATFORM_DELETED) {
          continue;
        }
        if ($environment->platform_status == HOSTING_PLATFORM_DELETED) {
          continue;
        }

        // Drush alias
        $environment->drush_alias = '@' . $environment->drush_alias;

        // Unserialize environment settings.
        $environment->settings = (object) unserialize($environment->settings);

        // Get last task
        $environment->last_task = node_load($environment->last_task_nid);

        // Get all tasks, keyed by type.
        $environment->tasks = devshop_get_tasks($environment);

        // Create tasks list, keyed by VID
        $environment->tasks_list = array();
        foreach ($environment->tasks as $task_type => $tasks) {
          foreach ($tasks as $task) {
            $environment->tasks_list[$task->vid] = $task;
          }
        }
        krsort($environment->tasks_list);

        // Detect creation method.
        // Some sites get installed. Others get imported.

        $environment->created = array();
        if (!empty($environment->tasks['install'])) {
          $environment->created['type'] = 'install';
          $environment->created['date'] = current($environment->tasks['install'])->executed;
          $environment->created['status'] = current($environment->tasks['install'])->task_status;
          $environment->created['nid'] = current($environment->tasks['install'])->nid;
          $environment->created['vid'] = current($environment->tasks['install'])->vid;
        }
        elseif (!empty($environment->tasks['import'])) {
          $environment->created['type'] = 'import';
          $environment->created['date'] = current($environment->tasks['import'])->executed;
          $environment->created['status'] = current($environment->tasks['import'])->task_status;
          $environment->created['nid'] = current($environment->tasks['import'])->nid;
          $environment->created['vid'] = current($environment->tasks['import'])->vid;
        }
        elseif (!empty($environment->tasks['clone'])) {
          $environment->created['type'] = 'clone';
          $environment->created['date'] = current($environment->tasks['clone'])->executed;
          $environment->created['status'] = current($environment->tasks['clone'])->task_status;
          $environment->created['nid'] = current($environment->tasks['clone'])->nid;
          $environment->created['vid'] = current($environment->tasks['clone'])->vid;
        }
        // Neither import nor install nor clone was found.
        else {
          $environment->created['type'] = '';
          $environment->created['date'] = '';
          $environment->created['status'] = '';
        }

        // Get the current git ref type.
        $environment->git_ref_type = $project->settings->git['refs'][$environment->git_ref];

        // The URL that the branch or tag links to.
        if ($project->git_provider == 'github') {
          $environment->git_ref_url = $project->git_repo_url . '/tree/' . $environment->git_ref;
        }
        else {
          $environment->git_ref_url = url("node/$environment->site/logs/commits");
        }

        // Environment Drupal version.
        $iid = db_query("SELECT iid FROM {hosting_package_instance} i left join {hosting_package} p on p.nid=i.package_id WHERE p.package_type = :type AND i.rid = :nid", array(':type' => 'platform', ':nid' => $environment->platform))->fetchField();
        $release = hosting_package_instance_load($iid);
        $environment->version = $release->version;

        // @TODO: Make "servers" more abstract so we can add many more.
        // HTTP Server
        $environment->servers['http'] = array(
          'nid' => $environment->web_server_nid,
          'name' => $environment->web_server,
        );

        // DB Server
        $environment->servers['db'] = array(
          'nid' => $environment->db_server,
          'name' => $environment->db_server,
        );

        // Web Server IP
        $server = node_load($environment->web_server_nid);
        $environment->ip_addresses = $server->ip_addresses;

        // Load Solr if present
        if (module_exists('hosting_solr')) {
          $server_nid = db_query("SELECT server FROM {hosting_solr} WHERE nid = :nid", array(':nid' => $environment->site))->fetchField();

          $servers = hosting_get_servers('solr');
          if ($server_nid && isset($servers[$server_nid])) {
            $environment->servers['solr'] = array(
              'nid' => $server_nid,
              'name' => $servers[$server_nid],
            );
          }
        }

        // Last commit
        $path = $project->code_path . '/' . $environment->name;
        if (file_exists($path)) {
          $environment->git_ref_id = shell_exec("cd $path; git log --pretty=format:'%ar' --max-count=1
      ");
          $environment->git_last = shell_exec("cd $path; git log --pretty=format:'%h %an %ar %s' --max-count=1");

          $environment->git_sha = trim(shell_exec("cd $path; git rev-parse HEAD"));
        }
        else {
          $environment->git_ref_id = '';
          $environment->git_last = '';
          $environment->git_sha = '';
        }

        // Repo root
        $environment->repo_root = empty($project->drupal_path) ? $environment->root : str_replace($project->drupal_path, '', $environment->root);

        // Load encryption status
        if (module_exists('hosting_ssl')) {
          $environment->ssl_enabled  = db_query('SELECT ssl_enabled FROM {hosting_ssl_site} WHERE nid = :nid', array(':nid' => $environment->site))->fetchField();
        }
        else {
          $environment->ssl_enabled = FALSE;
        }

        // Environments URLs
        // Get aegir "aliases"
        $environment->domains = hosting_alias_get_aliases($environment);
        $environment->domains[] = $environment->system_domain;
        $protocol = $environment->ssl_enabled ? 'https://' : 'http://';
        if ($environment->redirect_domain) {
          $environment->url = $protocol . $environment->redirect_domain;
        }
        else {
          $environment->url = $protocol . $environment->system_domain;
        }

        // If per-environment hooks config is not allowed, load it from the project.
        if (!$project->settings->deploy['allow_environment_deploy_config']) {
          $environment->settings->deploy = $project->settings->deploy['default_hooks'];
        }

        // Save to project environments collection.
        drupal_alter('devshop_environment', $environment, $project);
        $environments[$environment->name] = $environment;
      }
    }

    // Put live environment at the top.
    if (!empty($environments) && $project->settings->live['live_environment']) {
      $project->environments = array();
      $live_env = $project->settings->live['live_environment'];
      $project->environments[$live_env] = $environments[$live_env];
      unset($environments[$live_env]);
      $project->environments += $environments;
    }
    elseif (!empty($environments)) {
      $project->environments = $environments;
    }
    elseif (!empty($node->title)) {
      $project->environments = array();
    }

    // Make project name and status available.
    $project->name = $node->title;
    $project->status = $node->status;

    // Webhook Status
    // @TODO: Create Hosting Webhooks module.
    // @TODO: Remove devshop_pull.
    module_load_include('inc', 'devshop_pull');
    $project->webhook_url = _devshop_pull_callback_url($node);

    // Save project object be available at $node->project.
    $node->project = $project;

    // Load verification status of project node.
    $task = (object) db_query('SELECT * FROM {hosting_task} WHERE rid = :rid ORDER BY vid DESC', array(':rid' => $node->nid))->fetchAssoc();
    $task->task_status = (int) $task->task_status;
    $node->verify = $task;
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
  $info->install_profile = $project->install_profile;

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



/**
 * Implementation of hook_nodeapi()
 * For site and platform nodes, load project and environment objects.
 *
 * @see devshop_projects_load()
 */
function devshop_projects_nodeapi(&$node, $op, $a3 = null) {

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

  // Load environment info into tasks.
  if ($op == 'load' && $node->type == 'task') {
    $additions = array();
    $result = (object) db_query("SELECT nid, title AS name, name AS environment FROM {hosting_devshop_project_environment} e LEFT JOIN node ON e.project_nid = node.nid WHERE site = :site OR platform = :platform", array(
        ':site' => $node->rid,
        ':platform' => $node->rid
    ))->fetchAssoc();

    // Load project and environment
    $additions['project'] = $result;
    return $additions;
  }

  // When a task is updated, save the last_task status in the environment settings.
  if (($op == 'insert' || $op == 'update') && $node->type == 'task') {
    $site = node_load($node->rid);

    if (($site->type == 'site' || $site->type == 'platform') && isset($site->environment) && ($node->task_type == HOSTING_TASK_QUEUED || $node->task_type == HOSTING_TASK_PROCESSING)) {

      $site->environment->last_task = $node->nid;
      devshop_environment_save_last_task($site->environment);
    }
  }

  // When a task is created or updated that has a project, redirect to the project.
  if (($op == 'update' || $op == 'insert') && $node->type == 'task') {
    $object = node_load($node->rid);
    if (isset($object->project) && arg(0) == 'node' && arg(1) == $object->nid) {
      drupal_goto("node/{$object->project->nid}");

    }
  }

  // When a platform or site node is deleted, remove the environment record.
  if ($op == 'delete' && ($node->type == 'site' || $node->type == 'platform')) {
    drupal_set_message('Deleted environment.');
    db_delete('hosting_devshop_project_environment')
      ->condition($node->type, $node->nid)
      ->execute();
  }
}

/**
 * Implements hook_nodeapi_TYPE_OP() for site nodes and view op.
 */
function devshop_projects_nodeapi_task_load(&$node, $a3, $a4) {

  $task_types = hosting_available_tasks();
  $task_types = array_merge($task_types['site'], $task_types['platform']);

  // In Aegir, "executed" and "delta" does not get updated properly.
  // Handle "delta" being not computed yet
  if ($node->task_status == HOSTING_TASK_PROCESSING) {
    $additions['duration'] = format_interval(time() - $node->executed, 1);
  }
  elseif ($node->task_status == HOSTING_TASK_QUEUED) {
    $additions['duration'] = t('Queued for %time', array('%time' => format_interval(time() - $node->changed)));
  }
  else  {
    $additions['duration'] = format_interval($node->delta, 1);
  }

  // To test out constantly changing tasks, uncomment this line.
  //        $node->task_status = rand(-1, 3);
  $additions['status_class'] = devshop_task_status_class($node->task_status);
  $additions['status_name'] = _hosting_parse_error_code($node->task_status);

  // Override "ago" text.
  $additions['ago'] = format_interval(time() - $node->executed, 1) .' '. t('ago');
  if ($node->task_status == HOSTING_TASK_QUEUED) {
    $additions['ago'] = t('Queued for %time', array('%time' => format_interval(time() - $node->changed)));
  }
  elseif ($node->task_status == HOSTING_TASK_PROCESSING) {
    $additions['ago'] = format_interval(time() - $node->changed);

  }

  // Set task type
  $additions['type_name'] = $task_types[$node->task_type]['title'];

  // If platform's first verify...
  if ($node->ref->type == 'platform' && $node->ref->verified == 0) {
    $additions['type_name'] = t('Cloning Codebase');
  }

  // Get Icon
  $additions['icon'] = devshop_task_status_icon($node->task_status);

  // Get desired URL
  $additions['url'] = url("node/{$node->nid}");

  // @TODO: Doesn't quite work. we need a different VID for each clone link.
  //  if ($node->task_type == 'clone') {
  //    $additions['url'] = url("node/{$node->nid}/revisions/{$node->vid}/view");
  //  }
  return $additions;
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
