<?php

/**
* Implementation of hook_drush_command().
* Provides provision commands for all devshop tasks.
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
 * Command function for platform-delete.
 */
function drush_devshop_hosting_platform_delete($platform_path = NULL) {
    if (empty($platform_path)) {
        return drush_set_error(DRUSH_APPLICATION_ERROR, 'You must specify a platform path.');
    }
    drush_log('[DEVSHOP] Looking up platform at ' . $platform_path, 'ok');

    $nid = db_result(db_query('SELECT nid FROM {hosting_platform} WHERE publish_path ="%s"', $platform_path));

    if (empty($nid)) {
        return drush_set_error(DRUSH_APPLICATION_ERROR, 'No platform with that path was found.');
    }
    else {
        drush_log('[DEVSHOP] Found platform ' . $nid, 'ok');

        // Look for existing enabled sites.
        $site_nid = db_result(db_query('SELECT nid FROM {hosting_site} WHERE platform = %d AND status = %d', $nid, HOSTING_SITE_ENABLED));
        if (!empty($site_nid)) {
            return drush_set_error(DRUSH_APPLICATION_ERROR, "The platform at path $platform_path still has sites present. Please delete them first.");
        }

        if (drush_confirm(dt("Schedule the platform at $platform_path for deletion?"))){
            return hosting_add_task($nid, 'delete');
        }
    }
}