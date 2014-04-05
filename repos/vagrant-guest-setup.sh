#!/bin/sh
sudo su - aegir -c "
rm -rf /var/aegir/.drush/devshop_provision
ln -s /repos/devshop_provision /var/aegir/.drush/devshop_provision

rm -rf /var/aegir/devshop-6.x-1.x/profiles/devshop/modules/contrib/devshop_hosting
ln -s /repos/devshop_hosting /var/aegir/devshop-6.x-1.x/profiles/devshop/modules/devshop_hosting

rm -rf /var/aegir/.drush/provision_git
ln -s /repos/provision_git /var/aegir/.drush/provision_git
"