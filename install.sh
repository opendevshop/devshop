#!/bin/bash
#
#  DevShop Install Script
#  ======================
#
#  Install DevShop with Ansible.
#
#    $ sudo ./install.sh
#
#  This script installs Ansible.  For more information see http://ansible.com
#
echo "=================================="
echo " Welcome to the DevShop Installer"
echo "=================================="

# Fail if not running as root (sudo)
if [ $EUID -ne 0 ]; then
    echo "This script must be run as root.  Try 'sudo ./install.sh'." 1>&2
    exit 1
fi

# If ansible command is not available, install it.
if [ ! `which ansible` ]; then
    echo " Installing Ansible..."

    apt-get update -qq
    apt-get install -qq python-apt python-pycurl -y
    pip install ansible
    echo "----------------------------------"

else
    echo " Ansible already installed. Skipping installation."
    echo "----------------------------------"
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

echo "----------------------------------"
echo " Hostname: $HOSTNAME"
echo " MySQL Root Password: $MYSQL_ROOT_PASSWORD"
echo "----------------------------------"

if [ VAGRANT_CWD ]; then
  cd $VAGRANT_CWD installers/ansible

fi

cd installers/ansible
echo $HOSTNAME > inventory

# If ansible playbook fails syntax check, report it and exit.
if [[ ! `ansible-playbook -i inventory --syntax-check playbook.yml` ]]; then
    echo " Ansible syntax check failed! Check installers/ansible/playbook.yml and try again."
    exit 1
fi

# Run the playbook.
echo "----------------------------------"
echo " Installing with Ansible..."
echo "----------------------------------"

ansible-playbook -i inventory playbook.yml --connection=local --sudo --extra-vars "server_hostname=$HOSTNAME mysq_root_password=$MYSQL_ROOT_PASSWORD"
