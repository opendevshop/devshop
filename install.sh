#!/bin/bash
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
    --install-path       The path to install the main devshop source code including CLI, makefile, roles.yml (Default: /usr/share/devshop)
    --server-webserver   Set to 'nginx' if you want to use the Aegir NGINX packages. (Default: apache)
    --makefile           The makefile to use to build the front-end site. (Default: {install-path}/build-devmaster.make)
    --playbook           The Ansible playbook.yml file to use other than the included playbook.yml. (Default: {install-path}/playbook.yml)
    --email              The email address to use for User 1. Enter your email to receive notification when the install is complete.
    --aegir-uid          The UID to use for creating the `aegir` user (Default: 12345)
    --ansible-default-host-list  If your server is using a different ansible default host, specify it here. Default: /etc/ansible/hosts*
    --force-ansible-role-install   Specify option to pass the "--force" option to the `ansible-galaxy install` command, causing the script to overwrite existing roles. (Default: False)
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

Welcome to OpenDevShop! Use the link above to login.

You can run the command 'devshop login' to get another login link.
  
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

determine_os() {
    if [ -f '/etc/os-release' ]; then
        if . /etc/os-release; then
            OS=$ID
            VERSION="$VERSION_ID"
            HOSTNAME_FQDN=`hostname --fqdn`
        else
            echo "$FUNCNAME(): failed to import /etc/os-release" 1>&2
            return 1
        fi

    elif [ -f '/etc/lsb-release' ]; then
        if . /etc/lsb-release; then
            OS=$DISTRIB_ID
            VERSION="$DISTRIB_RELEASE"
            HOSTNAME_FQDN=`hostname --fqdn`

            if [ $OS == "Ubuntu" ]; then
              OS=ubuntu
            fi
        else
            echo "$FUNCNAME(): failed to import /etc/lsb-release" 1>&2
            return 1
        fi

    elif [ -f '/etc/redhat-release' ]; then
        OS=$(cat /etc/redhat-release | awk '{print tolower($1);}')
        VERSION=$(cat /etc/redhat-release | awk '{print $3;}')
        HOSTNAME_FQDN=`hostname --fqdn`
    else
        echo "$FUNCNAME(): failed to determine OS running" 1>&2
        return 1
    fi
}

determine_webserver() {
    # If /var/aegir/config/server_master/nginx.conf is found, use NGINX to install.
    # If /var/aegir/config/server_master/apache.conf is found, use apache to install.
    # This will override any selected option for web server. This is so we don't install
    # a second webserver accidentally.

    if [ -f "/var/aegir/config/server_master/nginx.conf" ]; then
      SERVER_WEBSERVER=nginx
      echo " An existing Aegir NGINX installation was found. Using 'nginx' for variable 'server-webserver'"
    elif [ -f "/var/aegir/config/server_master/apache.conf" ]; then
      SERVER_WEBSERVER=apache
      echo " An existing Aegir Apache installation was found. Using 'apache' for variable 'server-webserver'"
    fi
}

install_ansible() {
    echo " Installing Ansible..."

    if [ $OS == 'ubuntu' ] || [ $OS == 'debian' ]; then

        if [ $VERSION == '14.04' ]; then
            ANSIBLE_GALAXY_OPTIONS="$ANSIBLE_GALAXY_OPTIONS --ignore-certs"
        fi
         
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

        apt-get update -qq
        apt-get install $PACKAGE -y -qq
        apt-add-repository ppa:ansible/ansible -y
        apt-get update -qq
        apt-get install ansible -y -qq

    elif [ $OS == 'centos' ] || [ $OS == 'rhel' ] || [ $OS == 'redhat' ] || [ $OS == 'fedora'  ]; then

        # Build ansible from source to ensure the latest version.
        yum install -y git epel-release redhat-lsb-core > /dev/null 1>&1
        git clone http://github.com/ansible/ansible.git --recursive --branch stable-2.3

        # dir may not exist, or it may exist as a symlink.  lets handle this a little better.
        if ! [ -d "ansible" ]; then
          echo "The directory ansible does not exist which means git clone failed.  This could be a permission or link issue.  Check the referenced directory."
          exit 1
        else

          # Build ansible RPM from source code.
          yum install -y which rpm-build make asciidoc git python-setuptools python2-devel PyYAML python-httplib2 python-jinja2 python-keyczar python-paramiko python-six sshpass
          cd ansible
          git checkout v2.3.0.0-1
          make rpm > /dev/null 2>&1
          rpm -Uvh ./rpm-build/ansible-*.noarch.rpm

          ansible --version
        fi

        if [ ! `ansible --version` ]; then
          echo >&2 "We require ansible but it's not installed.  The installation has failed.  Aborting.";
          exit 1
        fi

    else
        echo "OS ($OS) is not known, or an install action was not understood.  Please post an issue with this message at http://github.com/opendevshop/devshop/issues/new"
        exit 1
    fi

    echo $LINE
}

install_distro_pkgs() {
    if [ $OS == 'ubuntu' ] || [ $OS == 'debian' ]; then
        apt-get -y install "$@"
    elif [ $OS == 'centos' ] || [ $OS == 'redhat' ] || [ $OS == 'fedora'  ]; then
        yum -y install "$@"
    else
        echo "$FUNCNAME(): don't know how to install packages" 1>&2
        return 1
    fi
}

install_required_distro_pkgs() {
    local -a pkgs_ar=( git )

    echo "Updating $OS's package index..."

    if [ $OS == 'centos' ] || [ $OS == 'redhat' ] || [ $OS == 'fedora'  ]; then
        pkgs_ar+=( epel-release  )

        if ! yum check-update; then
            echo "$FUNCNAME(): failed to get updates from the distribution's package index" 1>&2
            return 1
        fi
    elif [ $OS == 'ubuntu' ] || [ $OS == 'debian' ]; then
        if ! apt-get update -qq; then
            echo "$FUNCNAME(): failed to get updates from the distribution's package index" 1>&2
            return 1
        fi
    fi

    install_distro_pkgs "${pkgs_ar[@]}"
}

install_n_configure_devshop() {
    install_required_distro_pkgs || return $?

    if ! command -v ansible >/dev/null; then
        # If ansible command is not available, install it.
        # Decided on "hash" thanks to http://stackoverflow.com/questions/592620/check-if-a-program-exists-from-a-bash-script
        # After testing this thoroughly on centOS and ubuntu, I think we should use command -v
        install_ansible || return $?
    else
        echo " Ansible already installed. Skipping installation."
        echo $LINE
    fi

    # If --makefile option is not set, use DEVSHOP_INSTALL_PATH/build-devmaster.make
    if [ -z $MAKEFILE_PATH ]; then
      MAKEFILE_PATH="$DEVSHOP_INSTALL_PATH/build-devmaster.make"
    fi

    # Clone the installer code if a playbook path was not set.
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

    set_mysql_password || return $?

    echo $LINE
    echo " Hostname: $HOSTNAME_FQDN"
    echo " MySQL Root Password: $MYSQL_ROOT_PASSWORD"
    echo " Playbook: $DEVSHOP_INSTALL_PATH/playbook.yml "
    echo " Roles: $DEVSHOP_INSTALL_PATH/roles.yml "
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
      echo "Inventsory file found at $ANSIBLE_DEFAULT_HOST_LIST has $HOSTNAME_FQDN. Not modifying."
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
    ANSIBLE_EXTRA_VARS+=("devshop_version: ${DEVSHOP_VERSION}")
    ANSIBLE_EXTRA_VARS+=("aegir_user_uid: ${AEGIR_USER_UID}")
    ANSIBLE_EXTRA_VARS+=("travis: false")
    ANSIBLE_EXTRA_VARS+=("supervisor_running: true")

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
    echo " Installing ansible roles from $DEVSHOP_INSTALL_PATH/roles.yml in the ansible-galaxy default location..."
    ansible-galaxy install --force --ignore-errors --role-file "$DEVSHOP_INSTALL_PATH/roles.yml" $ANSIBLE_GALAXY_OPTIONS
    echo $LINE

    # Run the playbook.
    echo " Installing with Ansible..."
    echo $LINE

    if [ $SERVER_WEBSERVER == 'apache' ]; then
      PLAYBOOK_FILE="playbook.yml"
    elif [ $SERVER_WEBSERVER == 'nginx' ]; then
      PLAYBOOK_FILE="playbook-nginx.yml"
    fi

    # If ansible playbook fails syntax check, report it and exit.
    PLAYBOOK_PATH="$DEVSHOP_INSTALL_PATH/$PLAYBOOK_FILE"
    if [[ ! `ansible-playbook --syntax-check ${PLAYBOOK_PATH}` ]]; then
        echo " Ansible syntax check failed! Check ${PLAYBOOK_PATH} and try again."
        exit 1
    fi

    ansible-playbook $PLAYBOOK_PATH --connection=local $ANSIBLE_VERBOSITY
}

set_mysql_password() {
    if [ -f '/root/.my.cnf' ]
    then
      MYSQL_ROOT_PASSWORD=$(awk -F "=" '/pass/ {print $2}' /root/.my.cnf)
      echo " Password found at /root/.my.cnf, using $MYSQL_ROOT_PASSWORD"
    else
      MYSQL_ROOT_PASSWORD=$(< /dev/urandom tr -dc _A-Z-a-z-0-9 | head -c${2:-32};echo;)
      echo " Generating new MySQL root password... $MYSQL_ROOT_PASSWORD"
      echo $MYSQL_ROOT_PASSWORD > /tmp/mysql_root_password
    fi
}

# main

set -e

# Version used for cloning devshop playbooks
# Must be a branch or tag.
DEVSHOP_VERSION=1.x
DEVSHOP_INSTALL_PATH=/usr/share/devshop
SERVER_WEBSERVER=apache
MAKEFILE_PATH=''
AEGIR_USER_UID=${AEGIR_USER_UID:-12345}
ANSIBLE_VERBOSITY="";
ANSIBLE_GALAXY_OPTIONS=""
ANSIBLE_DEFAULT_HOST_LIST="/etc/ansible/hosts"
DEVSHOP_SUPPORT_LICENSE_KEY=""

export ANSIBLE_FORCE_COLOR=true

LINE=---------------------------------------------

DEVSHOP_SCRIPT_PATH=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
# The rest of the scripts are only cloned if the playbook path option is not found.
DEVSHOP_GIT_REPO='http://github.com/opendevshop/devshop.git'

# parse command line options
while [ $# -gt 0 ]; do
  case "$1" in
    --makefile=*)
      MAKEFILE_PATH="${1#*=}"
      ;;
    --server-webserver=*)
      SERVER_WEBSERVER="${1#*=}"
      # Fail if server-webserver is not apache or nginx
      if [ $SERVER_WEBSERVER != 'nginx' ] && [ $SERVER_WEBSERVER != 'apache' ]; then
          echo ' Invalid argument for --server-webserver. Must be nginx or apache.'
          exit 1
      fi
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
    --force-ansible-role-install)
      ANSIBLE_GALAXY_OPTIONS="$ANSIBLE_GALAXY_OPTIONS --force"
      shift # past argument
      ;;
    --license=*)
      DEVSHOP_SUPPORT_LICENSE_KEY="${1#*=}"
      ;;
    --ansible-default-host-list=*)
      ANSIBLE_DEFAULT_HOST_LIST="${1#*=}"
      ;;
    *)
      echo $LINE
      echo ' Invalid option.'
      echo $LINE
      exit 1
  esac
  shift
done

# Fail if not running as root (sudo)
if [ $EUID -ne 0 ]; then
    echo " This script must be run as root.  Try 'sudo -H bash install.sh'." 1>&2
    exit 1
fi

echo "============================================="
echo " Welcome to the DevShop Standalone Installer "
echo "                   v $DEVSHOP_VERSION        "
echo "============================================="

if determine_os; then
    # Output some info.
    echo " OS: $OS"
    echo " Version: $VERSION"
    echo " Hostname: $HOSTNAME_FQDN"
else
    echo "Error: failed to determine OS. DevSHOP installs only on Ubuntu," \
         " Debian, CentOS, Fedora and Redhat." 1>&2
    exit 1
fi

if determine_webserver; then
    # Notify user we are using the found webserver.
    # Output Web Server
    echo $LINE
    echo " Web Server: $SERVER_WEBSERVER"
    echo $LINE
fi

install_n_configure_devshop

# Run devshop status, return exit code.
su - aegir -c "devshop status"
if [ ${PIPESTATUS[0]} == 0 ]; then
  su - aegir -c "devshop login"
  echo "$POST_INSTALL_WELCOME_MSG"
  exit 0
else
  echo "The command 'devshop status' had an error. Check the logs and try again."
  exit 1
fi
