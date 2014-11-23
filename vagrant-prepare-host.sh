#!/bin/bash
#
# vagrant-prepare-host.sh
#
# Runs on your local machine (the vagrant host) to prepare source code for editing.
# This script is run automatically on `vagrant up`.  You do not need to this manually.
#

# Passed argument
cd $1

# Build a full devshop frontend on the host with drush make, with working-copy option.
if [ ! -d devshop-6.x-1.x ]; then
   drush make build-devshop.make devshop-6.x-1.x --working-copy --no-gitinfofile
   cp devshop-6.x-1.x/sites/default/default.settings.php devshop-6.x-1.x/sites/default/settings.php
   mkdir devshop-6.x-1.x/sites/devshop.local
   chmod 777 devshop-6.x-1.x/sites/devshop.local
fi

# Clone drush packages.
if [ ! -d drush ]; then
    mkdir drush
    git clone git@git.drupal.org:project/provision.git drush/provision --branch 6.x-2.x
    git clone git@git.drupal.org:project/provision_git.git drush/provision_git
    git clone git@git.drupal.org:project/devshop_provision.git drush/devshop_provision
    git clone git@git.drupal.org:project/provision_logs.git drush/provision_logs --branch 6.x-1.x
    git clone git@git.drupal.org:project/provision_logs.git drush/provision_tasks_extra --branch 6.x-1.x

fi