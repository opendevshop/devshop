#!/bin/sh

# Ask for drupal.org username for cloning repos.
if [ -z "$1" ]
  then
    echo "Missing username.  Add your drupal.org username as an argument to this script.  ie. 'sh init-repos.sh druplicon'"
    exit 0
fi

# DevShop Repos
`git clone $1@git.drupal.org:project/devshop.git`
`git clone $1@git.drupal.org:project/devshop_provision.git`
`git clone $1@git.drupal.org:project/devshop_hosting.git`
`git clone $1@git.drupal.org:project/provision_git.git`

# Run vagrant-guest-setup.sh on the guest machine
vagrant ssh -c 'sudo sh /vagrant/repos/vagrant-guest-setup.sh'