#!/bin/bash
if [ ! -d devshop-6.x-1.x ]; then
   drush make build-devshop.make devshop-6.x-1.x --working-copy --no-gitinfofile
   cp devshop-6.x-1.x/sites/default/default.settings.php devshop-6.x-1.x/sites/default/settings.php
fi

if [ ! -d drush ]; then
    mkdir drush
    git clone git@git.drupal.org:project/provision.git drush/provision --branch 6.x-2.x
    git clone git@git.drupal.org:project/provision_git.git drush/provision_git
    git clone git@git.drupal.org:project/devshop_provision.git drush/devshop_provision
    git clone git@git.drupal.org:project/provision_logs.git drush/provision_logs --branch 6.x-1.x
    git clone git@git.drupal.org:project/provision_logs.git drush/provision_tasks_extra --branch 6.x-1.x

fi