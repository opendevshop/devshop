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

# Version used for cloning devshop playbooks
# Must be a branch or tag.
DEVSHOP_VERSION=0.x
SERVER_WEBSERVER=apache

echo "============================================="
echo " Welcome to the DevShop Standalone Installer "
echo "                   v $DEVSHOP_VERSION        "
echo "============================================="


# The rest of the scripts are only cloned if the playbook path option is not found.
DEVSHOP_GIT_REPO='http://github.com/opendevshop/devshop.git'
DEVSHOP_SCRIPT_PATH=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

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

# If on travis, use localhost as the hostname
if [ "$TRAVIS" == "true" ]; then
  echo "TRAVIS DETECTED! Setting Hostname to 'localhost'."
  HOSTNAME_FQDN="localhost"
fi

LINE=---------------------------------------------

echo " OS: $OS"
echo " Version: $VERSION"
echo " Hostname: $HOSTNAME_FQDN"

# Detect playbook path option
while [ $# -gt 0 ]; do
  case "$1" in
    --playbook=*)
      PLAYBOOK_PATH="${1#*=}"
      ;;
    --server-webserver=*)
      SERVER_WEBSERVER="${1#*=}"
      ;;
    *)
      echo $LINE
      echo " Error: Invalid argument for --server-webserver."
      echo $LINE
      exit 1
  esac
  shift
done

# If /var/aegir/config/server_master/nginx.conf is found, use NGINX to install.
# If /var/aegir/config/server_master/apache.conf is found, use apache to install.

# This will override any selected option for web server. This is so we don't install
# a second webserver accidentally.

if [ -f "/var/aegir/config/server_master/nginx.conf" ]; then
  SERVER_WEBSERVER=nginx
elif [ -f "/var/aegir/config/server_master/apache.conf" ]; then
  SERVER_WEBSERVER=apache
fi

# Output Web Server
echo " Web Server: $SERVER_WEBSERVER"

if [ $PLAYBOOK_PATH ]; then
    :
# Detect playbook next to the install script
elif [ -f "$DEVSHOP_SCRIPT_PATH/playbook.yml" ]; then
    PLAYBOOK_PATH=$DEVSHOP_SCRIPT_PATH
else
    PLAYBOOK_PATH=/usr/share/devshop
fi

echo " Playbook: $PLAYBOOK_PATH/playbook.yml "
echo $LINE

# Notify user we are using the found webserver.
if [ -f "/var/aegir/config/server_master/nginx.conf" ]; then
  echo " An existing Aegir NGINX installation was found. Using 'nginx' for variable 'server_webserver'"
  echo $LINE
elif [ -f "/var/aegir/config/server_master/apache.conf" ]; then
  echo " An existing Aegir Apache installation was found. Using 'apache' for variable 'server_webserver'"
  echo $LINE
fi

# Fail if not running as root (sudo)
if [ $EUID -ne 0 ]; then
    echo " This script must be run as root.  Try 'sudo ./install.sh'." 1>&2
    exit 1
fi

# Fail if server_webserver is not apache or nginx
if [ $SERVER_WEBSERVER != 'nginx' ] && [ $SERVER_WEBSERVER != 'apache' ]; then
  echo ' Invalid Web server. Must be nginx or apache (default).'
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

        # @TODO: We should figure out how to add this to the playbook. It's tricky because of the lsb_release thing.
        if [ $SERVER_WEBSERVER == 'nginx' ]; then
            echo "deb http://ppa.launchpad.net/nginx/stable/ubuntu $(lsb_release -sc) main" | tee /etc/apt/sources.list.d/nginx-stable.list
            sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys C300EE8C
        fi

        apt-get update
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

MAKEFILE_PATH=''

# Generate MySQL Password
if [ "$TRAVIS" == "true" ]; then
  echo "TRAVIS DETECTED! Setting 'root' user password."
  MYSQL_ROOT_PASSWORD=''
  echo $MYSQL_ROOT_PASSWORD > /tmp/mysql_root_password
  MAKEFILE_PATH="https://raw.githubusercontent.com/opendevshop/devshop/$DEVSHOP_VERSION/build-devmaster.make"
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
if [ ! -f "$PLAYBOOK_PATH/playbook.yml" ]; then
  if [ ! -d "$PLAYBOOK_PATH" ]; then
    git clone $DEVSHOP_GIT_REPO $PLAYBOOK_PATH
    cd $PLAYBOOK_PATH
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

ANSIBLE_EXTRA_VARS="server_hostname=$HOSTNAME_FQDN mysql_root_password=$MYSQL_ROOT_PASSWORD playbook_path=$PLAYBOOK_PATH server_webserver=$SERVER_WEBSERVER"

if [ -n "$MAKEFILE_PATH" ]; then
  ANSIBLE_EXTRA_VARS="$ANSIBLE_EXTRA_VARS devshop_makefile=$MAKEFILE_PATH"
fi

ansible-playbook -i inventory playbook.yml --connection=local --extra-vars "$ANSIBLE_EXTRA_VARS"

# Run Composer install to enable devshop cli
cd $PLAYBOOK_PATH
composer install

# Run devshop status, return exit code.
su - aegir -c "devshop status"
if [ ${PIPESTATUS[0]} == 0 ]; then
  su - aegir -c "devshop login"
  echo ""
  echo "The command 'devshop status' ran successfully! Welcome to OpenDevShop!"
  echo ""
  exit 0
else
  echo "The command 'devshop status' had an error. Check the logs and try again."
  exit 1
fi
