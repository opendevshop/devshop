<?php

/**
 * Implements the provision-devshop-delete command.
 */
function drush_provision_provision_devshop_delete() {
  
  // Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  //exec('crontab -r');
  
  drush_log(dt('Project delete compkted ok!.'), 'ok');

  //Resume cron
  // @TODO Better way?  @see drush_hosting_pause()
  //provision_backend_invoke('hostmaster', 'hosting-setup');
}

