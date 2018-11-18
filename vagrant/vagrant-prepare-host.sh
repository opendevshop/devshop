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
cd ..

if [ ! -d aegir-home ]; then
  echo "Æ | Creating aegir-home directory..."
  mkdir aegir-home
fi

cd aegir-home

# Build a full devshop frontend on the host with drush make, with working-copy option.
if [ ! -d devmaster-$DEVMASTER_VERSION ]; then
   drush make $VAGRANT_HOME/build-devmaster.make devmaster-$DEVMASTER_VERSION --working-copy --no-gitinfofile
   cp devmaster-$DEVMASTER_VERSION/sites/default/default.settings.php devmaster-$DEVMASTER_VERSION/sites/default/settings.php
   mkdir devmaster-$DEVMASTER_VERSION/sites/devshop.site
   chmod 777 devmaster-$DEVMASTER_VERSION/sites -R
fi

# Clone drush packages.
if [ ! -d .drush ]; then
    echo "Æ | Creating .drush/commands folder..."
    mkdir -p .drush/commands
    cd .drush/commands
    echo "Æ | Cloning Provision..."
    git clone git@git.drupal.org:project/provision.git
    cd provision
    git checkout $AEGIR_VERSION

    cd ..
    echo "Æ | Cloning Registry Rebuild..."
    git clone git@git.drupal.org:project/registry_rebuild.git --branch 7.x-2.x
    cd ../..
fi

# Clone ansible roles.
cd $VAGRANT_HOME
if [ ! -d ../roles ]; then
    mkdir ../roles
    ansible-galaxy install -r ../roles.yml -p ../roles
    cd ../roles

    # Overwrite the roles installed by galaxy with git clones of Our Roles
    rm -rf opendevshop.aegir-user opendevshop.aegir-apache opendevshop.aegir-nginx opendevshop.devmaster opendevshop.devshop
    git clone git@github.com:opendevshop/ansible-role-aegir-user.git opendevshop.aegir-user
    git clone git@github.com:opendevshop/ansible-role-aegir-apache.git opendevshop.aegir-apache
    git clone git@github.com:opendevshop/ansible-role-aegir-nginx.git opendevshop.aegir-nginx
    git clone git@github.com:opendevshop/ansible-role-devmaster.git opendevshop.devmaster
    git clone git@github.com:opendevshop/ansible-role-devshop.git opendevshop.devshop

fi
