#!/usr/bin/env bash
#
usage() {

    echo \
'
  DevShop Standalone Install Script
  =================================

  This script will install a full devshop server from scratch.

  Please read the full "Installing DevShop" instructions at https://docs.opendevshop.com/install.html

  Before you start, please visit https://github.com/opendevshop/devshop/releases to be sure you have the latest version of this script,

  DNS & Hostnames
  ---------------
  For devshop to work, the server\''s hostname must be a fullly qualified domain name that resolves to an accessible IP address.

  Before you run this script, add DNS records for this server:

    devshop.mydomain.com. 1800 IN A 1.2.3.4
    *.devshop.mydomain.com. 1800 IN A 1.2.3.4

  This install script will attempt to set your hostname for you, if you use the --hostname option.


  Running Install.sh
  ==================

   Must run as root or with sudo and -H option:

    root@ubuntu:~# wget https://raw.githubusercontent.com/opendevshop/devshop/1.x/install.sh
    root@ubuntu:~# bash install.sh --hostname=devshop.mydomain.com

   OR

     sudo -H bash install.sh

  Options:
    --hostname           The desired fully qualified domain name to set as this machine\''s hostname (Default: Current hostname)
    --install-path       The path to install the main devshop source code. (Default: /usr/share/devshop)
    --server-webserver   Set to 'nginx' if you want to use the Aegir NGINX packages. (Default: apache)
    --makefile           The makefile to use to build the front-end site. (Default: {install-path}/build-devmaster.make)
    --playbook           The Ansible playbook.yml file to use other than the included playbook.yml. (Default: {install-path}/playbook.yml)
    --email              The email address to use for User 1. Enter your email to receive notification when the install is complete.
    --aegir-uid          The UID to use for creating the `aegir` user (Default: 12345)
    --ansible-default-host-list  If your server is using a different ansible default host, specify it here. Default: /etc/ansible/hosts*
    --license            The devshop.support license key for this server.
    --help               Displays this help message and exits

  Supporting DevShop
  ==================

  Your contributions make DevShop possible. Please consider becoming a patron of open source!

      https://opencollective.com/devshop
      https://www.patreon.com/devshop

'

  exit 1
}

# main

POST_INSTALL_WELCOME_MSG="

Welcome to OpenDevShop! Use the link below to sign in.

The password for user 'admin' was securely generated and hidden. 
Use `drush @hostmaster uli` or `devshop login` to get another login link.

Please visit http://getdevshop.com for help and information.

Join the development community at https://github.com/opendevshop/devshop

Thanks!

--The OpenDevShop Team

Issues: https://github.com/opendevshop/devshop/issues
Chat: https://gitter.im/opendevshop/devshop
Code: https://github.com/opendevshop/devshop

Your contributions make DevShop possible. Please consider becoming a patron of open source!

  https://opencollective.com/devshop
  https://www.patreon.com/devshop

"

# @TODO: Include all of the helpful things from get.docker.com.
# Simple way to run a command.
# Copied from get.docker.com
# Usage: $sh_c 'command_to_run'
sh_c='sh -c'
pip_packages="ansible pymysql"

set -e

# Version used for cloning devshop playbooks
# Must be a branch or tag.
DEVSHOP_VERSION=1.x
DEVSHOP_INSTALL_PATH=/usr/share/devshop
DEVSHOP_PLAYBOOK='playbook.yml'
SERVER_WEBSERVER=apache
MAKEFILE_PATH=''
AEGIR_USER_UID=${AEGIR_USER_UID:-12345}
ANSIBLE_VERBOSITY="";
ANSIBLE_DEFAULT_HOST_LIST="/etc/ansible/hosts"
DEVSHOP_SUPPORT_LICENSE_KEY=""

export ANSIBLE_FORCE_COLOR=true

echo "============================================="
echo " Welcome to the DevShop Standalone Installer "
echo "                   v $DEVSHOP_VERSION        "
echo "============================================="

# Fail if not running as root (sudo)
if [ $EUID -ne 0 ]; then
    echo " This script must be run as root.  Try 'sudo -H bash install.sh'." 1>&2
    exit 1
fi

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
elif [ -f '/etc/redhat-release' ]; then
    OS=$(cat /etc/redhat-release | awk '{print tolower($1);}')
    VERSION=$(cat /etc/redhat-release | awk '{print $3;}')
    HOSTNAME_FQDN=`hostname --fqdn`
fi

LINE=---------------------------------------------


# Detect playbook path option
while [ $# -gt 0 ]; do
  case "$1" in
    --makefile=*)
      MAKEFILE_PATH="${1#*=}"
      ;;
    --server-webserver=*)
      SERVER_WEBSERVER="${1#*=}"
      ;;
    --hostname=*)
      HOSTNAME_FQDN="${1#*=}"
      ;;
    --install-path=*)
      DEVSHOP_INSTALL_PATH="${1#*=}"
      ;;
    --email=*)
      DEVMASTER_ADMIN_EMAIL="${1#*=}"
      ;;
    --aegir-uid=*)
      AEGIR_USER_UID="${1#*=}"
      ;;
    -v|--verbose)
      ANSIBLE_VERBOSITY="-v"
      shift # past argument
      ;;
    -vvv|--very-verbose)
      ANSIBLE_VERBOSITY="-vvv"
      shift # past argument
      ;;
    -vvvv|--debug)
      ANSIBLE_VERBOSITY="-vvvv"
      shift # past argument
      ;;
    --license=*)
      DEVSHOP_SUPPORT_LICENSE_KEY="${1#*=}"
      ;;
    --ansible-default-host-list=*)
      ANSIBLE_DEFAULT_HOST_LIST="${1#*=}"
      ;;
    --playbook=*)
      DEVSHOP_PLAYBOOK="${1#*=}"
      ;;
    *)
      echo $LINE
      echo ' Invalid option.'
      echo $LINE
      exit 1
  esac
  shift
done

# Output some info.
echo " OS: $OS"
echo " Version: $VERSION"
echo " Hostname: $HOSTNAME_FQDN"
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

# If --makefile option is not set, use DEVSHOP_INSTALL_PATH/build-devmaster.make
if [ -z $MAKEFILE_PATH ]; then
  MAKEFILE_PATH="$DEVSHOP_INSTALL_PATH/build-devmaster.make"
fi

echo $LINE

# Notify user we are using the found webserver.
if [ -f "/var/aegir/config/server_master/nginx.conf" ]; then
  echo " An existing Aegir NGINX installation was found. Using 'nginx' for variable 'server-webserver'"
  echo $LINE
elif [ -f "/var/aegir/config/server_master/apache.conf" ]; then
  echo " An existing Aegir Apache installation was found. Using 'apache' for variable 'server-webserver'"
  echo $LINE
fi

# Fail if server-webserver is not apache or nginx
if [ $SERVER_WEBSERVER != 'nginx' ] && [ $SERVER_WEBSERVER != 'apache' ]; then
  echo ' Invalid argument for --server-webserver. Must be nginx or apache.'
  exit 1
fi

# If ansible command is not available, install it.
# Decided on "hash" thanks to http://stackoverflow.com/questions/592620/check-if-a-program-exists-from-a-bash-script
# After testing this thoroughly on centOS and ubuntu, I think we should use command -v

    echo " Preparing server prerequisites..."
    mkdir -p /etc/ansible

    if [ $OS == 'ubuntu' ] || [ $OS == 'debian' ]; then

        # Detect ubuntu version and switch package.
        # @TODO: Use the get_distribution() stuff from get.docker.com
        if [ $VERSION == '12.04' ]; then
      			pre_reqs="python-software-properties git"
        else
      			pre_reqs="software-properties-common git python3-setuptools python3-pip "
      	fi

        # @TODO: We should figure out how to add this to the playbook. It's tricky because of the lsb_release thing.
        if [ $SERVER_WEBSERVER == 'nginx' ]; then
            echo "deb http://ppa.launchpad.net/nginx/stable/ubuntu $(lsb_release -sc) main" | tee /etc/apt/sources.list.d/nginx-stable.list
            sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys C300EE8C
        fi

        # Copied from get.docker.com
        $sh_c 'apt-get update -qq >/dev/null'
				$sh_c "DEBIAN_FRONTEND=noninteractive apt-get install -y -qq $pre_reqs >/dev/null"
				$sh_c "pip3 install $pip_packages"
				$sh_c "update-alternatives --install /usr/bin/python python /usr/bin/python3 1"

    elif [ $OS == 'centos' ] || [ $OS == 'rhel' ] || [ $OS == 'redhat' ] || [ $OS == 'fedora'  ]; then

      # Copied from get.docker.com
      # @TODO: Might need epel-release first. See https://github.com/geerlingguy/docker-centos7-ansible/blob/master/Dockerfile
			if [ "$OS" = "fedora" ]; then
				pkg_manager="dnf"
				config_manager="dnf config-manager"
				enable_channel_flag="--set-enabled"
				disable_channel_flag="--set-disabled"
				pre_reqs="python-pip git sudo which"
				pkg_suffix="fc$dist_version"
			else
				pkg_manager="yum"
				config_manager="yum-config-manager"
				enable_channel_flag="--enable"
				disable_channel_flag="--disable"
				pre_reqs="python-pip git sudo which"
				pkg_suffix="el"
			fi

      # Duplicate steps in the core ansible Dockerfile (https://github.com/geerlingguy/docker-centos7-ansible/blob/master/Dockerfile)
      $sh_c "$pkg_manager makecache fast"
      $sh_c "$pkg_manager install -y -q deltarpm epel-release initscripts"
      $sh_c "$pkg_manager update -y"
      $sh_c "$pkg_manager install -y -q $pre_reqs"
			$sh_c "pip install $pip_packages"

    else
        echo "OS ($OS) is not known, or an install action was not understood.  Please post an issue with this message at http://github.com/opendevshop/devshop/issues/new"
        exit 1
    fi

    echo $LINE

ansible --version
python --version

if [ -f '/root/.my.cnf' ]
then
  MYSQL_ROOT_PASSWORD=$(awk -F "=" '/pass/ {print $2}' /root/.my.cnf)
  echo " Password found at /root/.my.cnf, using $MYSQL_ROOT_PASSWORD"
else
  MYSQL_ROOT_PASSWORD=$(< /dev/urandom tr -dc _A-Z-a-z-0-9 | head -c${2:-32};echo;)
  echo " Generating new MySQL root password... $MYSQL_ROOT_PASSWORD"
  echo $MYSQL_ROOT_PASSWORD > /tmp/mysql_root_password
fi

# Clone the installer if $DEVSHOP_INSTALL_PATH does not exist yet.
if [ ! -d "$DEVSHOP_INSTALL_PATH" ]; then
    git clone $DEVSHOP_GIT_REPO $DEVSHOP_INSTALL_PATH
    cd $DEVSHOP_INSTALL_PATH
    git checkout $DEVSHOP_VERSION
else
    cd $DEVSHOP_INSTALL_PATH
# @TODO: This was needed when install.sh was the upgrade, but now it isn't. It breaks upgrade testing in PRs
# unless DEVSHOP_VERSION is set to the branch being tested.
#    git fetch
#    git checkout $DEVSHOP_VERSION
fi

echo $LINE
echo " Hostname: $HOSTNAME_FQDN"
echo " MySQL Root Password: $MYSQL_ROOT_PASSWORD"
echo " Playbook: $DEVSHOP_INSTALL_PATH/$DEVSHOP_PLAYBOOK "
echo " Roles: $DEVSHOP_INSTALL_PATH/roles/"
echo " Makefile: $MAKEFILE_PATH "
echo $LINE


cd $DEVSHOP_INSTALL_PATH

# Check that DEFAULT_HOST_LIST ansible config matches ANSIBLE_DEFAULT_HOST_LIST
if [[ `ansible-config dump | grep ${ANSIBLE_DEFAULT_HOST_LIST}` ]]; then
  echo " Ansible Inventory: $ANSIBLE_DEFAULT_HOST_LIST"
else
  echo "[ERROR] The system's ansible configuration option DEFAULT_HOST_LIST does not match the install.sh option --ansible-default-host-list ($ANSIBLE_DEFAULT_HOST_LIST)"
  echo "Result from ansible-config dump | grep $ANSIBLE_DEFAULT_HOST_LIST:"
  ansible-config dump | grep $ANSIBLE_DEFAULT_HOST_LIST
  exit 1
fi

# Check inventory file for [devmaster] group or is executable, leave it alone.
if [ `cat ${ANSIBLE_DEFAULT_HOST_LIST} | grep ${HOSTNAME_FQDN}` ] || [[ -x "$ANSIBLE_DEFAULT_HOST_LIST" ]]; then
  echo "Inventory file found at $ANSIBLE_DEFAULT_HOST_LIST has $HOSTNAME_FQDN. Not modifying."
else
# Create inventory file.
  echo "Hostname $HOSTNAME_FQDN not found in the file $ANSIBLE_DEFAULT_HOST_LIST... Creating new file..."
  echo "[devmaster]" > $ANSIBLE_DEFAULT_HOST_LIST
  echo $HOSTNAME_FQDN >> $ANSIBLE_DEFAULT_HOST_LIST
fi

    # Create ansible vars files.
    #   ./group_vars/HOSTNAME: reserved for devshop control.
    #   ./hostname_vars/HOSTNAME: reserved for users to customize.
    ANSIBLE_CONFIG_PATH=$(dirname "${ANSIBLE_DEFAULT_HOST_LIST}")

ANSIBLE_VARS_HOSTNAME_PATH="$ANSIBLE_CONFIG_PATH/host_vars/$HOSTNAME_FQDN"
ANSIBLE_VARS_GROUP_PATH="$ANSIBLE_CONFIG_PATH/group_vars/devmaster"

# If Ansible host variables file is not found for this server, create the dir and write the file.
if [ ! -d "$ANSIBLE_CONFIG_PATH/host_vars" ]; then
  mkdir "$ANSIBLE_CONFIG_PATH/host_vars"
  echo "# Custom Variables for $HOSTNAME_FQDN." >> $ANSIBLE_VARS_HOSTNAME_PATH
  echo "# You may edit these variables which are used during the 'devshop verify' command." >> $ANSIBLE_VARS_HOSTNAME_PATH
  echo "# This file must be valid YML." >> $ANSIBLE_VARS_HOSTNAME_PATH
  echo "---" >> $ANSIBLE_VARS_HOSTNAME_PATH
  echo "name: value" >> $ANSIBLE_VARS_HOSTNAME_PATH
fi
# If Ansible group variables file is not found for this server, create the dir.
if [ ! -d "$ANSIBLE_CONFIG_PATH/group_vars" ]; then
  mkdir "$ANSIBLE_CONFIG_PATH/group_vars"
fi

# If Ansible.cfg file does not exist, copy it in.
if [ ! -f "$ANSIBLE_CONFIG_PATH/ansible.cfg" ]; then
  cp $DEVSHOP_INSTALL_PATH/ansible.cfg $ANSIBLE_CONFIG_PATH/ansible.cfg
fi

# Write to our devmaster group file every time install.sh is run."
# Strangest thing: if you leave a space after the variable "name:" the output will convert to a new line.
IFS=$'\n'
ANSIBLE_EXTRA_VARS=()
ANSIBLE_EXTRA_VARS+=("server_hostname: ${HOSTNAME_FQDN}")
ANSIBLE_EXTRA_VARS+=("devshop_cli_path: ${DEVSHOP_INSTALL_PATH}")
ANSIBLE_EXTRA_VARS+=("playbook_path: ${DEVSHOP_INSTALL_PATH}")
ANSIBLE_EXTRA_VARS+=("aegir_server_webserver: ${SERVER_WEBSERVER}")
# @TODO: Remove this var? a vars file gets created from this. We don't want old "devshop_version" vars around.
# ANSIBLE_EXTRA_VARS+=("devshop_version: ${DEVSHOP_VERSION}")
ANSIBLE_EXTRA_VARS+=("aegir_user_uid: ${AEGIR_USER_UID}")
ANSIBLE_EXTRA_VARS+=("devshop_github_token: ${GITHUB_TOKEN}")

# Lookup special variable overrides.
if [ -n "$MAKEFILE_PATH" ]; then
  ANSIBLE_EXTRA_VARS+=("devshop_makefile: ${MAKEFILE_PATH}")
fi
if [ -n "$DEVMASTER_ADMIN_EMAIL" ]; then
  ANSIBLE_EXTRA_VARS+=("devshop_devmaster_email: ${DEVMASTER_ADMIN_EMAIL}")
fi
if [ -n "$DEVSHOP_SUPPORT_LICENSE_KEY" ]; then
  ANSIBLE_EXTRA_VARS+=("devshop_support_license_key: ${DEVSHOP_SUPPORT_LICENSE_KEY}")
fi

# Render vars YML file
echo "# This variables file is written by devshop's install.sh script and 'devshop upgrade' command. Do not edit." > $ANSIBLE_VARS_GROUP_PATH
echo "# You may add variables to the host_vars file located at $ANSIBLE_VARS_HOSTNAME_PATH" >> $ANSIBLE_VARS_GROUP_PATH
echo "---" >> $ANSIBLE_VARS_GROUP_PATH
for i in ${ANSIBLE_EXTRA_VARS[@]}; do
    echo -e $i >> $ANSIBLE_VARS_GROUP_PATH
done

echo $LINE
echo "Wrote group variables file for devmaster to $ANSIBLE_VARS_GROUP_PATH"
echo $LINE

ANSIBLE_VARS_GROUP_MYSQL_PATH="$ANSIBLE_CONFIG_PATH/group_vars/mysql"
echo "# This variables file is written by devshop's install.sh script and 'devshop upgrade' command. Do not edit." > $ANSIBLE_VARS_GROUP_MYSQL_PATH
echo "# You may add variables to the host_vars file located at $ANSIBLE_VARS_HOSTNAME_PATH" >> $ANSIBLE_VARS_GROUP_MYSQL_PATH
echo "---" >> $ANSIBLE_VARS_GROUP_MYSQL_PATH
echo "mysql_root_password: $MYSQL_ROOT_PASSWORD" >> $ANSIBLE_VARS_GROUP_MYSQL_PATH

echo $LINE
echo "Wrote database secrets file for devmaster to $ANSIBLE_VARS_GROUP_MYSQL_PATH"
echo $LINE

# Run the playbook.
echo " Installing with Ansible..."
echo $LINE

# If ansible playbook fails syntax check, report it and exit.
PLAYBOOK_PATH="$DEVSHOP_INSTALL_PATH/$DEVSHOP_PLAYBOOK"
if [[ ! `ansible-playbook --syntax-check ${PLAYBOOK_PATH}` ]]; then
    echo " Ansible syntax check failed! Check ${PLAYBOOK_PATH} and try again."
    exit 1
fi

ansible-playbook $PLAYBOOK_PATH --connection=local --limit $HOSTNAME_FQDN $ANSIBLE_VERBOSITY

# Run devshop status, return exit code.
su - aegir -c "devshop status"
if [ ${PIPESTATUS[0]} == 0 ]; then
  echo $POST_INSTALL_WELCOME_MSG
  su - aegir -c "devshop login"
  exit 0
else
  echo "The command 'devshop status' had an error. Check the logs and try again."
  exit 1
fi


