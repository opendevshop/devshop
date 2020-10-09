<?php

/**
 * Implements hook_drush_command().
 * Provides the "platform-delete" command.  This command triggers a delete task on a platform specified by it's path.
 *
 * This allows taking the final step in an upgrade: deleting the old devmaster.
 */
function devshop_hosting_drush_command()
{
    $items['platform-delete'] = array(
        'description' => 'Trigger deletion of a platform. This is mainly used by the devshop upgrade task to delete the old hostmaster platform.',
        'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_SITE,
        'arguments' => array(
            'path' => 'The root path of the platform',
        ),
    );
    return $items;
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task()
 * for Verify tasks.
 *
 * Installs the "composer" plugin if needed, and then runs composer-manager install command.
 */
function devshop_hosting_post_hosting_verify_task($task, $data) {
  if ($task->ref->type == 'site' && $task->ref->hosting_name == 'hostmaster') {

    // Import SSH key,
    $stored_devshop_ssh_key = variable_get('devshop_public_key');
    if (empty(trim($stored_devshop_ssh_key))) {
      $server_public_key = file_get_contents($_SERVER['HOME'] . '/.ssh/id_rsa.pub');
      if (!empty($server_public_key)) {
        variable_set('devshop_public_key', $server_public_key);
        drush_log(dt('Loaded public key into DevShop.'), 'p_log');
      }
    }
  }
}

/**
 * Command function for platform-delete.
 */
function drush_devshop_hosting_platform_delete($platform_path = NULL) {
    if (empty($platform_path)) {
        return drush_set_error(DRUSH_APPLICATION_ERROR, 'You must specify a platform path.');
    }
    drush_log('[DEVSHOP] Looking up platform at ' . $platform_path, 'ok');

    $nid = db_query('SELECT nid FROM {hosting_platform} WHERE publish_path = :publish_path AND status = :status' , array(
      ':publish_path' => $platform_path,
      ':status' => HOSTING_PLATFORM_ENABLED,
    ))->fetchField();

    if (empty($nid)) {
        return drush_set_error(DRUSH_APPLICATION_ERROR, 'No platform with that path was found.');
    }
    else {
        drush_log('[DEVSHOP] Found platform ' . $nid, 'ok');

        // Look for existing enabled sites.
        $site_count = db_select('hosting_site')
          ->condition('platform', $nid)
          ->condition('status', HOSTING_SITE_ENABLED)
          ->fields('nid')
          ->countQuery()
          ->execute()
          ->fetchField();

        if (!empty($site_count)) {
            return drush_set_error(DRUSH_APPLICATION_ERROR, "The platform at path $platform_path still has sites present. Please delete them first.");
        }

        if (drush_confirm(dt("Schedule the platform at $platform_path for deletion?"))){
            return hosting_add_task($nid, 'delete');
        }
    }
}
