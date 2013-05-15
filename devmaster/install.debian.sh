#!/bin/bash
#  DevShop Install Script
#  ======================
#
#  Install DevShop in Debian based systems.
#
#  NOTE: Only thoroughly tested in Ubuntu Precise
#
#  To install, run the following command:
#
#    $ sudo ./install.debian.sh
#
echo "deb http://debian.aegirproject.org stable main" | tee -a /etc/apt/sources.list.d/aegir-stable.list
wget -q http://debian.aegirproject.org/key.asc -O- | apt-key add -
apt-get update
echo "aegir ALL=NOPASSWD: /usr/sbin/apache2ctl" | tee -a /etc/sudoers
apt-get install mysql-server -y
mysql_secure_installation
echo debconf aegir/makefile string http://drupalcode.org/project/devshop_provision.git/blob_plain/HEAD:/build-devshop.make | debconf-set-selections
echo debconf aegir/profile devshop | debconf-set-selections

# @TODO: How to get --profile option in place?
apt-get install aegir -y  
drush dl provision_git-6.x devshop_provision-6.x 
