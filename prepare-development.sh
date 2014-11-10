#!/bin/bash
if [ ! -d repos ]; then
    mkdir repos
    git clone git@git.drupal.org:project/devshop.git repos/devshop
    git clone git@git.drupal.org:project/devshop_provision.git repos/devshop_provision
    git clone git@git.drupal.org:project/devshop_hosting.git repos/devshop_hosting
    git clone git@git.drupal.org:project/provision_git.git repos/provision_git
    git clone git@git.drupal.org:project/provision.git repos/provision --branch 6.x-2.x
    git clone git@git.drupal.org:project/hosting.git repos/hosting --branch 6.x-2.x
    git clone git@git.drupal.org:project/hostmaster.git repos/hostmaster --branch 6.x-2.x
fi