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
 * for provision_deploy.
 *
 * This hook should be put in sites/all/drush of your project and can be used
 * to fire off anything you want, post deploy.
 *
 */
function drush_devshop_post_provision_deployEXAMPLE($branch = '') {

  drush_log('Hello, devshop', 'ok');

  if (empty($branch)) {
    $branch = d()->platform->git_ref;
  }

  // If no changes in files were detected, the context 'devshop_block_deploy_hooks' gets saved to TRUE.
  if (drush_get_context('devshop_block_deploy_hooks')){
//    return;
  }

  // If this is a site, log a message.
  if (d()->type == 'site'){

    $project_name = d()->project;
    $project = (object) d("@project_{$project_name}")->project;
    $environment = (object) $project->environments[d()->hosting_environment];

    drush_log("[CUSTOM] Successfully checked out $branch to environment $environment->name in project $project_name.", 'ok');

//    provision_backend_invoke(d()->name, 'status');

  }
}
