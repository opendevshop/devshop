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

  // Exported permission: 'create devshop-deploy task'.
  $permissions['create devshop-deploy task'] = array(
    'name' => 'create devshop-deploy task',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_projects',
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

  // Exported permission: 'create project'.
  $permissions['create project'] = array(
    'name' => 'create project',
    'roles' => array(
      'administrator' => 'administrator',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'devshop_projects',
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

  // Exported permission: 'delete own client'.
  $permissions['delete own client'] = array(
    'name' => 'delete own client',
    'roles' => array(
      'administrator' => 'administrator',
      'aegir administrator' => 'aegir administrator',
    ),
    'module' => 'hosting_client',
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
