#!/bin/bash
#
#  DevShop Standalone Install Script
#  =================================
#
#  Install DevShop with Ansible.
#
#  To clone devshop playbook from source:
#
#    $ sudo ./install.sh
#
#  To use a local playbook: (use the directory path, do not include playbook.yml)
#
#    $ sudo ./install.sh /path/to/playbook
#
#  For example, if using vagrant:
#
#    $ sudo ./install.sh /vagrant/installers/ansible
#
#  If using a playbook path option, the makefile used to build devmaster is defined
#  in the vars.yml file: devshop_makefile.
#
echo "============================================="
echo " Welcome to the DevShop Standalone Installer "
echo "============================================="

# Version used for cloning devshop playbooks
# Must be a branch or tag.
DEVSHOP_VERSION=0.2.0

# The rest of the scripts are only cloned if the playbook path option is not found.
DEVSHOP_GIT_REPO='http://github.com/opendevshop/devshop.git'

if [ -f '/etc/os-release' ]; then
    . /etc/os-release
    OS=$ID
    VERSION="$VERSION_ID"
    HOSTNAME_FQDN=`hostname --fqdn`

elif [ -f '/etc/lsb-release' ]; then
    . /etc/lsb-release
    OS=$DISTRIB_ID
    VERSION="$DISTRIB_RELEASE"
    HOSTNAME_FQDN=`hostname --fqdn`

    if [ $OS == "Ubuntu" ]; then
      OS=ubuntu
    fi
fi

LINE=---------------------------------------------

echo " OS: $OS"
echo " Version: $VERSION"
echo " Hostname: $HOSTNAME_FQDN"
echo $LINE

# Detect playbook path option
if [ $1 ]; then
    PLAYBOOK_PATH=$1
    echo " Using playbook $1/playbook.yml "
    echo $LINE
else
    PLAYBOOK_PATH=/usr/share/devshop
fi

# Fail if not running as root (sudo)
if [ $EUID -ne 0 ]; then
    echo "This script must be run as root.  Try 'sudo ./install.sh'." 1>&2
    exit 1
fi

# If ansible command is not available, install it.
if [ ! `which ansible` ]; then
    echo " Installing Ansible..."

    if [ $OS == 'ubuntu' ] || [ $OS == 'debian' ]; then

        # Detect ubuntu version and switch package.
        if [ $VERSION == '12.04' ]; then
            PACKAGE=python-software-properties
        else
            PACKAGE=software-properties-common
        fi

        apt-get install git -y
        apt-get install $PACKAGE -y
        apt-add-repository ppa:ansible/ansible -y
        apt-get update
        apt-get install ansible -y

    elif [ $OS == 'centos' ] || [ $OS == 'redhat' ] || [ $OS == 'fedora'  ]; then

        yum install git -y
        yum install epel-release -y
        yum install ansible -y
    fi

    echo $LINE

else
    echo " Ansible already installed. Skipping installation."
    echo $LINE
fi

# Generate MySQL Password
if [ "$TRAVIS" == "true" ]; then
  echo "TRAVIS DETECTED! Setting 'root' user password."
  MYSQL_ROOT_PASSWORD=''
  echo $MYSQL_ROOT_PASSWORD > /tmp/mysql_root_password
fi

if [ -f '/root/.my.cnf' ]
then
  MYSQL_ROOT_PASSWORD=$(awk -F "=" '/pass/ {print $2}' /root/.my.cnf)
  echo " Password found at /root/.my.cnf, using $MYSQL_ROOT_PASSWORD"
else
  MYSQL_ROOT_PASSWORD=$(< /dev/urandom tr -dc _A-Z-a-z-0-9 | head -c${2:-32};echo;)
  echo " Generating new MySQL root password... $MYSQL_ROOT_PASSWORD"
  echo $MYSQL_ROOT_PASSWORD > /tmp/mysql_root_password
fi

echo $LINE
echo " Hostname: $HOSTNAME_FQDN"
echo " MySQL Root Password: $MYSQL_ROOT_PASSWORD"
echo $LINE

# Clone the installer code if a playbook path was not set.
MAKEFILE_PATH=''
if [ ! -f "$PLAYBOOK_PATH/playbook.yml" ]; then
  if [ ! -d "$PLAYBOOK_PATH" ]; then
    git clone $DEVSHOP_GIT_REPO $PLAYBOOK_PATH
    git checkout $DEVSHOP_VERSION
  else
    cd $PLAYBOOK_PATH
    git pull
  fi
  PLAYBOOK_PATH=/usr/share/devshop
  MAKEFILE_PATH=/usr/share/devshop/build-devmaster.make
  echo $LINE

fi

cd $PLAYBOOK_PATH

# Create inventory file
echo $HOSTNAME_FQDN > inventory

# If ansible playbook fails syntax check, report it and exit.
if [[ ! `ansible-playbook -i inventory --syntax-check playbook.yml` ]]; then
    echo " Ansible syntax check failed! Check installers/ansible/playbook.yml and try again."
    exit 1
fi

# Run the playbook.
echo " Installing with Ansible..."
echo $LINE

ANSIBLE_EXTRA_VARS="server_hostname=$HOSTNAME_FQDN mysql_root_password=$MYSQL_ROOT_PASSWORD playbook_path=$PLAYBOOK_PATH"

if [ -n "$MAKEFILE_PATH" ]; then
  ANSIBLE_EXTRA_VARS="$ANSIBLE_EXTRA_VARS devshop_makefile=$MAKEFILE_PATH"
fi

ansible-playbook -i inventory playbook.yml --connection=local --sudo --extra-vars "$ANSIBLE_EXTRA_VARS"

# DevShop Installed!
if [  ! -f '/var/aegir/.drush/hostmaster.alias.drushrc.php' ]; then

  echo "╔═════════════════════════════════════════════════════════════════════╗"
  echo "║ It appears something failed during installation.                    ║"
  echo "║ There is no '/var/aegir/.drush/hostmaster.alias.drushrc.php' file.  ║"
  echo "╚═════════════════════════════════════════════════════════════════════╝"
  exit 1
else

  echo "╔═══════════════════════════════════════════════════════════════╗"
  echo "║           ____  Welcome to  ____  _                           ║"
  echo "║          |  _ \  _____   __/ ___|| |__   ___  _ __            ║"
  echo "║          | | | |/ _ \ \ / /\___ \| '_ \ / _ \| '_ \           ║"
  echo "║          | |_| |  __/\ V /  ___) | | | | (_) | |_) |          ║"
  echo "║          |____/ \___| \_/  |____/|_| |_|\___/| .__/           ║"
  echo "║                                              |_|   v 0.2.0    ║"
  echo "╟───────────────────────────────────────────────────────────────╢"
  echo "║ Submit any issues to                                          ║"
  echo "║ http://drupal.org/node/add/project-issue/devshop              ║"
  echo "╟───────────────────────────────────────────────────────────────╢"
  echo "║ NOTES                                                         ║"
  echo "║ Your MySQL root password was set as a long secure string.     ║"
  echo "║ It was saved at /root/.my.cnf                                 ║"
  echo "║                                                               ║"
  echo "║ An SSH keypair has been created in /var/aegir/.ssh            ║"
  echo "║                                                               ║"
  echo "║ Supervisor is running Hosting Queue Runner.                   ║"
  echo "╠═══════════════════════════════════════════════════════════════╣"
  echo "║ Use this link to login:                               ║"
  echo "╚═══════════════════════════════════════════════════════════════╝"
  sudo su - aegir -c "drush @hostmaster uli"
fi
