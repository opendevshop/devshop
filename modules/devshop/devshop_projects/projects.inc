<?php

/**
 * @param $entities
 * @param $type
 */
function devshop_projects_entity_load($entities, $type) {

  if ($type == 'node') {

    $types = array(
      'site',
      'platform',
      'task',
    );

    foreach ($entities as &$entity) {
      if (in_array($entity->type, $types)) {

        // Load project and environment
        if ($entity->type == 'task') {

          // If this is a Verify Project task, we don't want to load anything.
          if (strpos($entity->title, 'Verify Project:') === 0) {
            $entity->project = $entity->rid;
            $entity->environment = NULL;
            continue;
          }

          $nid = $entity->rid;
        }
        else {
          $nid = $entity->nid;
        }

        $query = db_query("
SELECT
  project_nid, name FROM {hosting_devshop_project_environment}
  WHERE site  = :site
    OR platform = :platform
    OR project_nid = :project
", array(
          ':site' => $nid,
          ':platform' => $nid,
          ':project' => $nid,
        ));

        $result = $query->fetch();

        // Only load the project node if there is no task
        if ($entity->type == 'task') {
          $entity->project = $result->project_nid;
          $entity->environment = $result->name;
        }
        else if (isset($result->project_nid)) {
          $project_node = node_load($result->project_nid);
          if (!empty($project_node->project)) {
            $entity->project = $project_node->project;
            if (isset($project_node->project->environments[$result->name])) {
              $entity->environment = $project_node->project->environments[$result->name];
            }
          }
        }
      }
    }
  }
}

/**
 * Load a project object.
 */
function devshop_project_load($node) {

  if (empty($node->nid) || $node->type != 'project') {
    return;
  }

  // Load verification status of project node.
  $query = db_query('SELECT * FROM {hosting_task} WHERE rid = :rid ORDER BY vid DESC', array(':rid' => $node->nid));
  $task = $query->fetch();

  // Load project data
  $project = (object) db_query('SELECT * FROM {hosting_devshop_project} WHERE nid = :nid', array(':nid' => $node->nid))->fetchAssoc();

  // Load up all project settings.
  $project->name = $node->project_name;
  $project->settings = (object) unserialize($project->settings);

  if (!empty($task)) {
    $project->verify = $task;
    $task->task_status = (int) $task->task_status;
  }

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
    $environment->last_task_node = node_load($environment->last_task_nid);

    // Get all tasks, keyed by type.
    $environment->tasks = devshop_get_tasks($environment);

    // Create tasks list, keyed by VID
    $environment->tasks_list = array();
    foreach ($environment->tasks as $task_type => $tasks) {
      foreach ($tasks as $task) {
        $environment->tasks_list[$task->nid] = $task;
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

    if (isset($release->version)) {
      $environment->version = $release->version;
    }
    else {
      $environment->version = t('unknown');
    }

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
    if (!empty($server)) {
      $environment->ip_addresses = $server->ip_addresses;
    }
    else {
      $environment->ip_addresses = array();
    }

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

    // Git information.
    $environment->git_ref_stored = $environment->git_ref;
    $path = $project->code_path . '/' . $environment->name;
    if (file_exists($path)) {

      // Timestamp of last commit.
      $environment->git_last = shell_exec("cd $path; git log --pretty=format:'%ar' --max-count=1");

      // The last commit.
      $environment->git_commit = shell_exec("cd $path; git -c color.ui=always log --max-count=1");

      // Get the exact SHA
      $environment->git_sha = trim(shell_exec("cd $path; git rev-parse HEAD"));

      // Get the actual tag or branch
      $environment->git_ref = trim(str_replace('refs/heads/', '', shell_exec("cd $path; git describe --tags --exact-match || git symbolic-ref -q HEAD")));

      // Get git status.
      $environment->git_status = trim(shell_exec("cd $path; git -c color.ui=always  status"));

      // Get git diff.
      $environment->git_diff = trim(shell_exec("cd $path; git -c color.ui=always diff"));

    }
    else {
      $environment->git_last = '';
      $environment->git_commit = '';
      $environment->git_sha = '';
      $environment->git_status = '';
      $environment->git_diff = '';
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
    if (isset($project->settings->deploy) && !$project->settings->deploy['allow_environment_deploy_config']) {
      $environment->settings->deploy = $project->settings->deploy['default_hooks'];
    }

    // Save to project environments collection.
    drupal_alter('devshop_environment', $environment, $project);
    $environments[$environment->name] = $environment;
  }

  // Put live environment at the top.
  if (!empty($environments) && isset($project->settings->live) && $project->settings->live['live_environment']) {
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
  return $project;

}
