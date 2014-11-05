#!/bin/bash
#
#  DevShop Standalone Install Script
#  =================================
#
#  Install DevShop with Ansible.
#
#    $ sudo ./install.sh
#
#  This script installs Ansible.  For more information see http://ansible.com
#
echo "============================================="
echo " Welcome to the DevShop Standalone Installer "
echo "============================================="

. /etc/lsb-release
OS=$DISTRIB_ID
VER=$DISTRIB_RELEASE

LINE=---------------------------------------------

echo " OS: $DISTRIB_ID"
echo " Version: $DISTRIB_RELEASE"
echo $LINE

# Fail if not running as root (sudo)
if [ $EUID -ne 0 ]; then
    echo "This script must be run as root.  Try 'sudo ./install.sh'." 1>&2
    exit 1
fi

# If ansible command is not available, install it.
if [ ! `which ansible` ]; then
    echo " Installing Ansible..."

    # Detect ubuntu version and switch package.
    if [ $DISTRIB_RELEASE == '14.04' ]; then
        PACKAGE=software-properties-common
    else
        PACKAGE=python-software-properties
    fi

    apt-get install $PACKAGE git -y
    apt-add-repository ppa:ansible/ansible -y
    apt-get update
    apt-get install ansible -y

    echo $LINE

else
    echo " Ansible already installed. Skipping installation."
    echo $LINE
fi

# Generate our attributes
if [ -f '/tmp/mysql_root_password' ]
then
  MYSQL_ROOT_PASSWORD=$(cat /tmp/mysql_root_password)
  echo "Password found, using $MYSQL_ROOT_PASSWORD"
else
  MYSQL_ROOT_PASSWORD=$(< /dev/urandom tr -dc _A-Z-a-z-0-9 | head -c${1:-32};echo;)
  echo "Generating new MySQL root password... $MYSQL_ROOT_PASSWORD"
  echo $MYSQL_ROOT_PASSWORD > /tmp/mysql_root_password
fi

echo $LINE
echo " Hostname: $HOSTNAME"
echo " MySQL Root Password: $MYSQL_ROOT_PASSWORD"
echo $LINE

# Clone the installer code
mkdir /tmp/devshop-install
cd /tmp/devshop-install
git clone http://git.drupal.org/project/devshop.git
cd devshop/installers/ansible

# Create inventory file
echo $HOSTNAME > inventory

# If ansible playbook fails syntax check, report it and exit.
if [[ ! `ansible-playbook -i inventory --syntax-check playbook.yml` ]]; then
    echo " Ansible syntax check failed! Check installers/ansible/playbook.yml and try again."
    exit 1
fi

# Run the playbook.
echo $LINE
echo " Installing with Ansible..."
echo $LINE

ansible-playbook -i inventory playbook.yml --connection=local --sudo --extra-vars "server_hostname=$HOSTNAME mysql_root_password=$MYSQL_ROOT_PASSWORD"
