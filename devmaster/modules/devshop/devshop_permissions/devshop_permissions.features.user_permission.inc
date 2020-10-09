<?php

/**
 * @file
 * devshop_permissions.features.user_permission.inc
 */

/**
 * Implements hook_user_default_permissions().
 */
function devshop_permissions_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'access content'.
  $permissions['access content'] = array(
    'name' => 'access content',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir account manager' => 'aegir account manager',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'node',
  );

  // Exported permission: 'access content overview'.
  $permissions['access content overview'] = array(
    'name' => 'access content overview',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'node',
  );

  // Exported permission: 'access disabled sites'.
  $permissions['access disabled sites'] = array(
    'name' => 'access disabled sites',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'anonymous user' => 'anonymous user',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting',
  );

  // Exported permission: 'access filemanager'.
  $permissions['access filemanager'] = array(
    'name' => 'access filemanager',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_filemanager',
  );

  // Exported permission: 'access hosting logs'.
  $permissions['access hosting logs'] = array(
    'name' => 'access hosting logs',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_logs',
  );

  // Exported permission: 'access hosting wizard'.
  $permissions['access hosting wizard'] = array(
    'name' => 'access hosting wizard',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting',
  );

  // Exported permission: 'access task logs'.
  $permissions['access task logs'] = array(
    'name' => 'access task logs',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'access test results'.
  $permissions['access test results'] = array(
    'name' => 'access test results',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_testing',
  );

  // Exported permission: 'add remote aliases to projects'.
  $permissions['add remote aliases to projects'] = array(
    'name' => 'add remote aliases to projects',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_remotes',
  );

  // Exported permission: 'administer SSH public keys'.
  $permissions['administer SSH public keys'] = array(
    'name' => 'administer SSH public keys',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'sshkey',
  );

  // Exported permission: 'administer clients'.
  $permissions['administer clients'] = array(
    'name' => 'administer clients',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir account manager' => 'aegir account manager',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
    ),
    'module' => 'hosting_client',
  );

  // Exported permission: 'administer content types'.
  $permissions['administer content types'] = array(
    'name' => 'administer content types',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'node',
  );

  // Exported permission: 'administer hosting'.
  $permissions['administer hosting'] = array(
    'name' => 'administer hosting',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting',
  );

  // Exported permission: 'administer hosting aliases'.
  $permissions['administer hosting aliases'] = array(
    'name' => 'administer hosting aliases',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_alias',
  );

  // Exported permission: 'administer hosting backup queue'.
  $permissions['administer hosting backup queue'] = array(
    'name' => 'administer hosting backup queue',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_backup_queue',
  );

  // Exported permission: 'administer hosting features'.
  $permissions['administer hosting features'] = array(
    'name' => 'administer hosting features',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting',
  );

  // Exported permission: 'administer hosting queues'.
  $permissions['administer hosting queues'] = array(
    'name' => 'administer hosting queues',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting',
  );

  // Exported permission: 'administer hosting settings'.
  $permissions['administer hosting settings'] = array(
    'name' => 'administer hosting settings',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting',
  );

  // Exported permission: 'administer hosting site backup manager'.
  $permissions['administer hosting site backup manager'] = array(
    'name' => 'administer hosting site backup manager',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_site_backup_manager',
  );

  // Exported permission: 'administer nodes'.
  $permissions['administer nodes'] = array(
    'name' => 'administer nodes',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'node',
  );

  // Exported permission: 'administer platforms'.
  $permissions['administer platforms'] = array(
    'name' => 'administer platforms',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_platform',
  );

  // Exported permission: 'administer projects'.
  $permissions['administer projects'] = array(
    'name' => 'administer projects',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_projects',
  );

  // Exported permission: 'administer sites'.
  $permissions['administer sites'] = array(
    'name' => 'administer sites',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_site',
  );

  // Exported permission: 'administer tasks'.
  $permissions['administer tasks'] = array(
    'name' => 'administer tasks',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'bypass node access'.
  $permissions['bypass node access'] = array(
    'name' => 'bypass node access',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'node',
  );

  // Exported permission: 'cancel own tasks'.
  $permissions['cancel own tasks'] = array(
    'name' => 'cancel own tasks',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'change site domain name'.
  $permissions['change site domain name'] = array(
    'name' => 'change site domain name',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_projects',
  );

  // Exported permission: 'configure devshop pull'.
  $permissions['configure devshop pull'] = array(
    'name' => 'configure devshop pull',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_pull',
  );

  // Exported permission: 'create backup schedules'.
  $permissions['create backup schedules'] = array(
    'name' => 'create backup schedules',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_backup_queue',
  );

  // Exported permission: 'create backup task'.
  $permissions['create backup task'] = array(
    'name' => 'create backup task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'create backup-delete task'.
  $permissions['create backup-delete task'] = array(
    'name' => 'create backup-delete task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'create client'.
  $permissions['create client'] = array(
    'name' => 'create client',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir account manager' => 'aegir account manager',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'hosting_client',
  );

  // Exported permission: 'create clone task'.
  $permissions['create clone task'] = array(
    'name' => 'create clone task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_clone',
  );

  // Exported permission: 'create config_export task'.
  $permissions['create config_export task'] = array(
    'name' => 'create config_export task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'aegir_config',
  );

  // Exported permission: 'create config_import task'.
  $permissions['create config_import task'] = array(
    'name' => 'create config_import task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'aegir_config',
  );

  // Exported permission: 'create delete task'.
  $permissions['create delete task'] = array(
    'name' => 'create delete task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'create devshop-delete task'.
  $permissions['create devshop-delete task'] = array(
    'name' => 'create devshop-delete task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_projects',
  );

  // Exported permission: 'create deploy task'.
  $permissions['create deploy task'] = array(
    'name' => 'create deploy task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_site',
  );

  // Exported permission: 'create disable task'.
  $permissions['create disable task'] = array(
    'name' => 'create disable task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'create download task'.
  $permissions['create download task'] = array(
    'name' => 'create download task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'aegir_download',
  );

  // Exported permission: 'create enable task'.
  $permissions['create enable task'] = array(
    'name' => 'create enable task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'create features_revert_all task'.
  $permissions['create features_revert_all task'] = array(
    'name' => 'create features_revert_all task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_tasks_extra',
  );

  // Exported permission: 'create features_update_all task'.
  $permissions['create features_update_all task'] = array(
    'name' => 'create features_update_all task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_tasks_extra',
  );

  // Exported permission: 'create flush_cache task'.
  $permissions['create flush_cache task'] = array(
    'name' => 'create flush_cache task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_tasks_extra',
  );

  // Exported permission: 'create flush_drush_cache task'.
  $permissions['create flush_drush_cache task'] = array(
    'name' => 'create flush_drush_cache task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_tasks_extra',
  );

  // Exported permission: 'create git-pull task'.
  $permissions['create git-pull task'] = array(
    'name' => 'create git-pull task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir account manager' => 'aegir account manager',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_git_pull',
  );

  // Exported permission: 'create lock task'.
  $permissions['create lock task'] = array(
    'name' => 'create lock task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'create login-reset task'.
  $permissions['create login-reset task'] = array(
    'name' => 'create login-reset task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'create migrate task'.
  $permissions['create migrate task'] = array(
    'name' => 'create migrate task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_migrate',
  );

  // Exported permission: 'create package'.
  $permissions['create package'] = array(
    'name' => 'create package',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'hosting_package',
  );

  // Exported permission: 'create platform'.
  $permissions['create platform'] = array(
    'name' => 'create platform',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_platform',
  );

  // Exported permission: 'create platform git-checkout task'.
  $permissions['create platform git-checkout task'] = array(
    'name' => 'create platform git-checkout task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_git_checkout',
  );

  // Exported permission: 'create platform git-commit task'.
  $permissions['create platform git-commit task'] = array(
    'name' => 'create platform git-commit task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_git_commit',
  );

  // Exported permission: 'create platform git-pull task'.
  $permissions['create platform git-pull task'] = array(
    'name' => 'create platform git-pull task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_git_pull',
  );

  // Exported permission: 'create platform git-tag task'.
  $permissions['create platform git-tag task'] = array(
    'name' => 'create platform git-tag task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_git_tag',
  );

  // Exported permission: 'create project'.
  $permissions['create project'] = array(
    'name' => 'create project',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_projects',
  );

  // Exported permission: 'create project content'.
  $permissions['create project content'] = array(
    'name' => 'create project content',
    'roles' => array(
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'node',
  );

  // Exported permission: 'create rebuild_registry task'.
  $permissions['create rebuild_registry task'] = array(
    'name' => 'create rebuild_registry task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_tasks_extra',
  );

  // Exported permission: 'create restore task'.
  $permissions['create restore task'] = array(
    'name' => 'create restore task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'create run_cron task'.
  $permissions['create run_cron task'] = array(
    'name' => 'create run_cron task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_tasks_extra',
  );

  // Exported permission: 'create server'.
  $permissions['create server'] = array(
    'name' => 'create server',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'hosting_server',
  );

  // Exported permission: 'create site'.
  $permissions['create site'] = array(
    'name' => 'create site',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_site',
  );

  // Exported permission: 'create site aliases'.
  $permissions['create site aliases'] = array(
    'name' => 'create site aliases',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_alias',
  );

  // Exported permission: 'create site git-checkout task'.
  $permissions['create site git-checkout task'] = array(
    'name' => 'create site git-checkout task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir account manager' => 'aegir account manager',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_git_checkout',
  );

  // Exported permission: 'create site git-commit task'.
  $permissions['create site git-commit task'] = array(
    'name' => 'create site git-commit task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir account manager' => 'aegir account manager',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_git_commit',
  );

  // Exported permission: 'create site git-tag task'.
  $permissions['create site git-tag task'] = array(
    'name' => 'create site git-tag task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir account manager' => 'aegir account manager',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_git_tag',
  );

  // Exported permission: 'create sites on locked platforms'.
  $permissions['create sites on locked platforms'] = array(
    'name' => 'create sites on locked platforms',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_platform',
  );

  // Exported permission: 'create sync task'.
  $permissions['create sync task'] = array(
    'name' => 'create sync task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_sync',
  );

  // Exported permission: 'create test task'.
  $permissions['create test task'] = array(
    'name' => 'create test task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_testing',
  );

  // Exported permission: 'create unlock task'.
  $permissions['create unlock task'] = array(
    'name' => 'create unlock task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'create update task'.
  $permissions['create update task'] = array(
    'name' => 'create update task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_tasks_extra',
  );

  // Exported permission: 'create update_drupal task'.
  $permissions['create update_drupal task'] = array(
    'name' => 'create update_drupal task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'aegir_update',
  );

  // Exported permission: 'create update_translations task'.
  $permissions['create update_translations task'] = array(
    'name' => 'create update_translations task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_tasks_extra',
  );

  // Exported permission: 'create verify task'.
  $permissions['create verify task'] = array(
    'name' => 'create verify task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'delete any project content'.
  $permissions['delete any project content'] = array(
    'name' => 'delete any project content',
    'roles' => array(),
    'module' => 'node',
  );

  // Exported permission: 'delete own client'.
  $permissions['delete own client'] = array(
    'name' => 'delete own client',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'hosting_client',
  );

  // Exported permission: 'delete own project content'.
  $permissions['delete own project content'] = array(
    'name' => 'delete own project content',
    'roles' => array(),
    'module' => 'node',
  );

  // Exported permission: 'delete package'.
  $permissions['delete package'] = array(
    'name' => 'delete package',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'hosting_package',
  );

  // Exported permission: 'delete platform'.
  $permissions['delete platform'] = array(
    'name' => 'delete platform',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_platform',
  );

  // Exported permission: 'delete projects'.
  $permissions['delete projects'] = array(
    'name' => 'delete projects',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_projects',
  );

  // Exported permission: 'delete revisions'.
  $permissions['delete revisions'] = array(
    'name' => 'delete revisions',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'node',
  );

  // Exported permission: 'delete server'.
  $permissions['delete server'] = array(
    'name' => 'delete server',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'hosting_server',
  );

  // Exported permission: 'delete site'.
  $permissions['delete site'] = array(
    'name' => 'delete site',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_site',
  );

  // Exported permission: 'edit any project content'.
  $permissions['edit any project content'] = array(
    'name' => 'edit any project content',
    'roles' => array(),
    'module' => 'node',
  );

  // Exported permission: 'edit client uname'.
  $permissions['edit client uname'] = array(
    'name' => 'edit client uname',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir account manager' => 'aegir account manager',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'hosting_client',
  );

  // Exported permission: 'edit client users'.
  $permissions['edit client users'] = array(
    'name' => 'edit client users',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir account manager' => 'aegir account manager',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'hosting_client',
  );

  // Exported permission: 'edit own client'.
  $permissions['edit own client'] = array(
    'name' => 'edit own client',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
    ),
    'module' => 'hosting_client',
  );

  // Exported permission: 'edit own project content'.
  $permissions['edit own project content'] = array(
    'name' => 'edit own project content',
    'roles' => array(
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'node',
  );

  // Exported permission: 'edit package'.
  $permissions['edit package'] = array(
    'name' => 'edit package',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'hosting_package',
  );

  // Exported permission: 'edit platform'.
  $permissions['edit platform'] = array(
    'name' => 'edit platform',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_platform',
  );

  // Exported permission: 'edit project'.
  $permissions['edit project'] = array(
    'name' => 'edit project',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_projects',
  );

  // Exported permission: 'edit server'.
  $permissions['edit server'] = array(
    'name' => 'edit server',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'hosting_server',
  );

  // Exported permission: 'edit site'.
  $permissions['edit site'] = array(
    'name' => 'edit site',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_site',
  );

  // Exported permission: 'manage any SSH public keys'.
  $permissions['manage any SSH public keys'] = array(
    'name' => 'manage any SSH public keys',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'sshkey',
  );

  // Exported permission: 'manage own SSH public keys'.
  $permissions['manage own SSH public keys'] = array(
    'name' => 'manage own SSH public keys',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'sshkey',
  );

  // Exported permission: 'manage site HTTPS settings'.
  $permissions['manage site HTTPS settings'] = array(
    'name' => 'manage site HTTPS settings',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
    ),
    'module' => 'hosting_https',
  );

  // Exported permission: 'remove remote aliases from projects'.
  $permissions['remove remote aliases from projects'] = array(
    'name' => 'remove remote aliases from projects',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_remotes',
  );

  // Exported permission: 'retry failed tasks'.
  $permissions['retry failed tasks'] = array(
    'name' => 'retry failed tasks',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'revert revisions'.
  $permissions['revert revisions'] = array(
    'name' => 'revert revisions',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'node',
  );

  // Exported permission: 'update status of tasks'.
  $permissions['update status of tasks'] = array(
    'name' => 'update status of tasks',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'view any SSH public keys'.
  $permissions['view any SSH public keys'] = array(
    'name' => 'view any SSH public keys',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'sshkey',
  );

  // Exported permission: 'view client'.
  $permissions['view client'] = array(
    'name' => 'view client',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir account manager' => 'aegir account manager',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
    ),
    'module' => 'hosting_client',
  );

  // Exported permission: 'view locked platforms'.
  $permissions['view locked platforms'] = array(
    'name' => 'view locked platforms',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_platform',
  );

  // Exported permission: 'view own SSH public keys'.
  $permissions['view own SSH public keys'] = array(
    'name' => 'view own SSH public keys',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'sshkey',
  );

  // Exported permission: 'view own tasks'.
  $permissions['view own tasks'] = array(
    'name' => 'view own tasks',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  // Exported permission: 'view own unpublished content'.
  $permissions['view own unpublished content'] = array(
    'name' => 'view own unpublished content',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'node',
  );

  // Exported permission: 'view package'.
  $permissions['view package'] = array(
    'name' => 'view package',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
    ),
    'module' => 'hosting_package',
  );

  // Exported permission: 'view platform'.
  $permissions['view platform'] = array(
    'name' => 'view platform',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_platform',
  );

  // Exported permission: 'view project'.
  $permissions['view project'] = array(
    'name' => 'view project',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_projects',
  );

  // Exported permission: 'view projects'.
  $permissions['view projects'] = array(
    'name' => 'view projects',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_projects',
  );

  // Exported permission: 'view revisions'.
  $permissions['view revisions'] = array(
    'name' => 'view revisions',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
    ),
    'module' => 'node',
  );

  // Exported permission: 'view server'.
  $permissions['view server'] = array(
    'name' => 'view server',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_server',
  );

  // Exported permission: 'view site'.
  $permissions['view site'] = array(
    'name' => 'view site',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir account manager' => 'aegir account manager',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_site',
  );

  // Exported permission: 'view task'.
  $permissions['view task'] = array(
    'name' => 'view task',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
      'aegir client' => 'aegir client',
      'aegir platform manager' => 'aegir platform manager',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'hosting_task',
  );

  return $permissions;
}
