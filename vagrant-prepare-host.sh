#!/bin/bash
#
# vagrant-prepare-host.sh
#
# Runs on your local machine (the vagrant host) to prepare source code for editing.
# This script is run automatically on `vagrant up`.  You do not need to this manually.
#

# Passed argument is Vagrant Home folder.
VAGRANT_HOME=$1
DEVMASTER_VERSION=$2
cd $1

if [ ! -d source ]; then
  mkdir source
fi

cd source

# Build a full devshop frontend on the host with drush make, with working-copy option.
if [ ! -d devmaster-$DEVMASTER_VERSION ]; then
   drush make $VAGRANT_HOME/build-devmaster.make devmaster-$DEVMASTER_VERSION --working-copy --no-gitinfofile
   cp devmaster-$DEVMASTER_VERSION/sites/default/default.settings.php devmaster-$DEVMASTER_VERSION/sites/default/settings.php
   mkdir devmaster-$DEVMASTER_VERSION/sites/devshop.site
   chmod 777 devmaster-$DEVMASTER_VERSION/sites/devshop.site
fi

# Clone drush packages.
if [ ! -d drush ]; then
    mkdir drush
    cd drush
#    git clone git@github.com:opendevshop/devshop_provision.git --branch 6.x-2.x
    git clone git@git.drupal.org:project/provision.git --branch 7.x-3.x
#    git clone git@git.drupal.org:project/provision_solr.git --branch 6.x-2.x
    git clone git@git.drupal.org:project/registry_rebuild.git --branch 6.x-2.x
fi