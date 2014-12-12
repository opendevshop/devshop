<?php
/**
 * @file devshop.drush.inc
 * Site specific hooks.
 *
 * Put this file in sites/all/drush of your project to fire after deployment.
 *
 */

/**
 * Implements drush_HOOK_post_COMMAND()
 * for provision_devshop_deploy.
 *
 */
function drush_devshop_post_provision_devshop_deploy($branch) {
  
  drush_log('Hello, devshop', 'ok');

  // If this is a site, log a message.
  if (d()->type == 'site'){
    $environment = (object) d()->environment;
    $environment_name = $environment->name;
    $project_name = d()->project;

    drush_log("[CUSTOM] Successfully checked out $branch to environment $environment_name in project $project_name.", 'ok');

    drush_invoke('status');
  }
}
