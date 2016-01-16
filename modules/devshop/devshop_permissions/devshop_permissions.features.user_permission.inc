<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function devshop_permissions_user_default_permissions() {
  $permissions = array();

  // Exported permission: access content
  $permissions['access content'] = array(
    'name' => 'access content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: access disabled sites
  $permissions['access disabled sites'] = array(
    'name' => 'access disabled sites',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
    ),
  );

  // Exported permission: access filemanager
  $permissions['access filemanager'] = array(
    'name' => 'access filemanager',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: access hosting logs
  $permissions['access hosting logs'] = array(
    'name' => 'access hosting logs',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: access hosting wizard
  $permissions['access hosting wizard'] = array(
    'name' => 'access hosting wizard',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: access project logs
  $permissions['access project logs'] = array(
    'name' => 'access project logs',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: access task logs
  $permissions['access task logs'] = array(
    'name' => 'access task logs',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer SSH public keys
  $permissions['administer SSH public keys'] = array(
    'name' => 'administer SSH public keys',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer clients
  $permissions['administer clients'] = array(
    'name' => 'administer clients',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer hosting
  $permissions['administer hosting'] = array(
    'name' => 'administer hosting',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer hosting aliases
  $permissions['administer hosting aliases'] = array(
    'name' => 'administer hosting aliases',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer hosting backup queue
  $permissions['administer hosting backup queue'] = array(
    'name' => 'administer hosting backup queue',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer hosting features
  $permissions['administer hosting features'] = array(
    'name' => 'administer hosting features',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer hosting queues
  $permissions['administer hosting queues'] = array(
    'name' => 'administer hosting queues',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer hosting settings
  $permissions['administer hosting settings'] = array(
    'name' => 'administer hosting settings',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer hosting site backup manager
  $permissions['administer hosting site backup manager'] = array(
    'name' => 'administer hosting site backup manager',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer platforms
  $permissions['administer platforms'] = array(
    'name' => 'administer platforms',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer projects
  $permissions['administer projects'] = array(
    'name' => 'administer projects',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer servers
  $permissions['administer servers'] = array(
    'name' => 'administer servers',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer sites
  $permissions['administer sites'] = array(
    'name' => 'administer sites',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: administer tasks
  $permissions['administer tasks'] = array(
    'name' => 'administer tasks',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: cancel own tasks
  $permissions['cancel own tasks'] = array(
    'name' => 'cancel own tasks',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: configure devshop pull
  $permissions['configure devshop pull'] = array(
    'name' => 'configure devshop pull',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create backup schedules
  $permissions['create backup schedules'] = array(
    'name' => 'create backup schedules',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create backup task
  $permissions['create backup task'] = array(
    'name' => 'create backup task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create backup-delete task
  $permissions['create backup-delete task'] = array(
    'name' => 'create backup-delete task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create client
  $permissions['create client'] = array(
    'name' => 'create client',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: create clone task
  $permissions['create clone task'] = array(
    'name' => 'create clone task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create delete task
  $permissions['create delete task'] = array(
    'name' => 'create delete task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create devshop-commit task
  $permissions['create devshop-commit task'] = array(
    'name' => 'create devshop-commit task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create devshop-create task
  $permissions['create devshop-create task'] = array(
    'name' => 'create devshop-create task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create devshop-delete task
  $permissions['create devshop-delete task'] = array(
    'name' => 'create devshop-delete task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create devshop-deploy task
  $permissions['create devshop-deploy task'] = array(
    'name' => 'create devshop-deploy task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create disable task
  $permissions['create disable task'] = array(
    'name' => 'create disable task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create download task
  $permissions['create download task'] = array(
    'name' => 'create download task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create enable task
  $permissions['create enable task'] = array(
    'name' => 'create enable task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create flush_cache task
  $permissions['create flush_cache task'] = array(
    'name' => 'create flush_cache task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create flush_drush_cache task
  $permissions['create flush_drush_cache task'] = array(
    'name' => 'create flush_drush_cache task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create lock task
  $permissions['create lock task'] = array(
    'name' => 'create lock task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create login-reset task
  $permissions['create login-reset task'] = array(
    'name' => 'create login-reset task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create migrate task
  $permissions['create migrate task'] = array(
    'name' => 'create migrate task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create package
  $permissions['create package'] = array(
    'name' => 'create package',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: create platform
  $permissions['create platform'] = array(
    'name' => 'create platform',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create project
  $permissions['create project'] = array(
    'name' => 'create project',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create rebuild_registry task
  $permissions['create rebuild_registry task'] = array(
    'name' => 'create rebuild_registry task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create restore task
  $permissions['create restore task'] = array(
    'name' => 'create restore task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create run_cron task
  $permissions['create run_cron task'] = array(
    'name' => 'create run_cron task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create server
  $permissions['create server'] = array(
    'name' => 'create server',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: create site
  $permissions['create site'] = array(
    'name' => 'create site',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create site aliases
  $permissions['create site aliases'] = array(
    'name' => 'create site aliases',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create sites on locked platforms
  $permissions['create sites on locked platforms'] = array(
    'name' => 'create sites on locked platforms',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create sync task
  $permissions['create sync task'] = array(
    'name' => 'create sync task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create unlock task
  $permissions['create unlock task'] = array(
    'name' => 'create unlock task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create update task
  $permissions['create update task'] = array(
    'name' => 'create update task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: create verify task
  $permissions['create verify task'] = array(
    'name' => 'create verify task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: delete own client
  $permissions['delete own client'] = array(
    'name' => 'delete own client',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete package
  $permissions['delete package'] = array(
    'name' => 'delete package',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete platform
  $permissions['delete platform'] = array(
    'name' => 'delete platform',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: delete projects
  $permissions['delete projects'] = array(
    'name' => 'delete projects',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: delete server
  $permissions['delete server'] = array(
    'name' => 'delete server',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete site
  $permissions['delete site'] = array(
    'name' => 'delete site',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: edit client uname
  $permissions['edit client uname'] = array(
    'name' => 'edit client uname',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: edit client users
  $permissions['edit client users'] = array(
    'name' => 'edit client users',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: edit own client
  $permissions['edit own client'] = array(
    'name' => 'edit own client',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: edit package
  $permissions['edit package'] = array(
    'name' => 'edit package',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: edit platform
  $permissions['edit platform'] = array(
    'name' => 'edit platform',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: edit project
  $permissions['edit project'] = array(
    'name' => 'edit project',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: edit server
  $permissions['edit server'] = array(
    'name' => 'edit server',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: edit site
  $permissions['edit site'] = array(
    'name' => 'edit site',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: manage any SSH public keys
  $permissions['manage any SSH public keys'] = array(
    'name' => 'manage any SSH public keys',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: manage own SSH public keys
  $permissions['manage own SSH public keys'] = array(
    'name' => 'manage own SSH public keys',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: retry failed tasks
  $permissions['retry failed tasks'] = array(
    'name' => 'retry failed tasks',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: update status of tasks
  $permissions['update status of tasks'] = array(
    'name' => 'update status of tasks',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: view any SSH public keys
  $permissions['view any SSH public keys'] = array(
    'name' => 'view any SSH public keys',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: view client
  $permissions['view client'] = array(
    'name' => 'view client',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: view git commit logs
  $permissions['view git commit logs'] = array(
    'name' => 'view git commit logs',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: view locked platforms
  $permissions['view locked platforms'] = array(
    'name' => 'view locked platforms',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: view own SSH public keys
  $permissions['view own SSH public keys'] = array(
    'name' => 'view own SSH public keys',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: view own tasks
  $permissions['view own tasks'] = array(
    'name' => 'view own tasks',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: view package
  $permissions['view package'] = array(
    'name' => 'view package',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: view platform
  $permissions['view platform'] = array(
    'name' => 'view platform',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: view project
  $permissions['view project'] = array(
    'name' => 'view project',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: view projects
  $permissions['view projects'] = array(
    'name' => 'view projects',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: view server
  $permissions['view server'] = array(
    'name' => 'view server',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: view site
  $permissions['view site'] = array(
    'name' => 'view site',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: view task
  $permissions['view task'] = array(
    'name' => 'view task',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  return $permissions;
}
