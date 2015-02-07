#!/bin/bash
#
# vagrant-prepare-host.sh
#
# Runs on your local machine (the vagrant host) to prepare source code for editing.
# This script is run automatically on `vagrant up`.  You do not need to this manually.
#

# Passed argument is Vagrant Home folder.
VAGRANT_HOME=$1
cd $1

if [ ! -d source ]; then
  mkdir source
fi

cd source

# Build a full devshop frontend on the host with drush make, with working-copy option.
if [ ! -d devmaster-7.x-1.x ]; then
   drush make $VAGRANT_HOME/build-devmaster.make devmaster-7.x-1.x --working-copy --no-gitinfofile
   cp devmaster-7.x-1.x/sites/default/default.settings.php devmaster-7.x-1.x/sites/default/settings.php
   mkdir devmaster-7.x-1.x/sites/devshop.local
   chmod 777 devmaster-7.x-1.x/sites/devshop.local
fi

# Clone drush packages.
if [ ! -d drush ]; then
    mkdir drush
    cd drush
    git clone git@git.drupal.org:project/devshop_provision.git --branch 7.x-3.x
    git clone git@git.drupal.org:project/provision.git --branch 7.x-3.x
#    git clone git@git.drupal.org:project/provision_git.git --branch 6.x-1.x
#    git clone git@git.drupal.org:project/provision_logs.git --branch 6.x-1.x
#    git clone git@git.drupal.org:project/provision_site_backup_manager.git --branch 6.x-2.x
#    git clone git@git.drupal.org:project/provision_solr.git --branch 6.x-2.x
#    git clone git@git.drupal.org:project/provision_tasks_extra.git --branch 6.x-2.x
    git clone git@git.drupal.org:project/registry_rebuild.git --branch 7.x-2.x
fi