#!/usr/bin/env bash
set -e
# OpenDevShop for Linux installation script
#
# See https://opendevshop.com/install/ for the installation steps.
#
# This script is meant for quick & easy install via:
#   $ curl -fsSL https://get.devshop.tech -o get-devshop.sh
#   $ sh get-devshop.sh
#
# If you have not yet set a hostname, you can do so with the install script:
#
#   $ sh get-devshop.sh --hostname=devshop.example.com
#

# NOTE: Make sure to verify the contents of the script
#       you downloaded matches the contents of install.sh
#       located at https://github.com/opendevshop/devshop/blob/1.x/install/install.sh
#       before executing.
#

# OPTIONS

# Git commit from https://github.com/opendevshop/devshop/blob/1.x/install/install.sh when
# the script was uploaded (Should only be modified by upload job):
# Will be the SHA used to publish the install.sh file to get.devshop.tech.
SCRIPT_COMMIT_SHA="${LOAD_SCRIPT_COMMIT_SHA}"

# Version to install (branch or tag).
# If testing a branch is needed, set the DEVSHOP_VERSION environment variable in
# the command line environment:
#
#     $ export DEVSHOP_VERSION=bug/XXX/fix
#     $ bash install.sh
#
# See the GitHub action ./.github/workflows/install.yml
DEVSHOP_VERSION=${DEVSHOP_VERSION:-1.x}

# Version of Ansible to install
ANSIBLE_VERSION=${ANSIBLE_VERSION:-"2.9"}
pip_packages="ansible==${ANSIBLE_VERSION}"

# Git repo to install.
DEFAULT_DOWNLOAD_URL="http://github.com/opendevshop/devshop.git"
if [ -z "$DOWNLOAD_URL" ]; then
    DOWNLOAD_URL=$DEFAULT_DOWNLOAD_URL
fi

# Environment Options:
HOSTNAME_FQDN=${HOSTNAME_FQDN:-`hostname --fqdn`}
ANSIBLE_DEFAULT_HOST_LIST=${ANSIBLE_DEFAULT_HOST_LIST:-"/etc/ansible/hosts"}
DEVSHOP_INSTALL_PATH=${DEVSHOP_INSTALL_PATH:-"/usr/share/devshop"}
DEVSHOP_PLAYBOOK=${DEVSHOP_PLAYBOOK:-"roles/devshop.server/play.yml"}
SERVER_WEBSERVER=${SERVER_WEBSERVER:-"apache"}
AEGIR_USER_UID=${AEGIR_USER_UID:-12345}
ANSIBLE_VERBOSITY=${ANSIBLE_VERBOSITY:-12345};
DEVSHOP_SUPPORT_LICENSE_KEY=${DEVSHOP_SUPPORT_LICENSE_KEY:-""};

# Command line Options
while [ $# -gt 0 ]; do
    case "$1" in
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
        --dry-run)
            DRY_RUN=1
            ;;
        --*)
            echo "Illegal option $1"
            ;;
    esac
    shift $(( $# > 0 ? 1 : 0 ))
done

# Initial Ansible Variables
# Generate host specific vars to be injected into inventory.
# All command line options that are ansible variables should be saved here.
ANSIBLE_EXTRA_VARS=()
# This is to saved to local /etc/ansible/hosts file, so always use local connect.
ANSIBLE_EXTRA_VARS+=("ansible_host: ${HOSTNAME_FQDN}")
ANSIBLE_EXTRA_VARS+=("ansible_connection: local")
ANSIBLE_EXTRA_VARS+=("server_hostname: ${HOSTNAME_FQDN}")
ANSIBLE_EXTRA_VARS+=("devshop_control_git_reference: ${DEVSHOP_VERSION}")
ANSIBLE_EXTRA_VARS+=("devshop_cli_path: ${DEVSHOP_INSTALL_PATH}")
ANSIBLE_EXTRA_VARS+=("aegir_server_webserver: ${SERVER_WEBSERVER}")
ANSIBLE_EXTRA_VARS+=("aegir_user_uid: ${AEGIR_USER_UID}")
ANSIBLE_EXTRA_VARS+=("devshop_github_token: ${GITHUB_TOKEN}")
if [ -n "$DEVMASTER_ADMIN_EMAIL" ]; then
  ANSIBLE_EXTRA_VARS+=("devshop_devmaster_email: ${DEVMASTER_ADMIN_EMAIL}")
fi
if [ -n "$DEVSHOP_SUPPORT_LICENSE_KEY" ]; then
  ANSIBLE_EXTRA_VARS+=("devshop_support_license_key: ${DEVSHOP_SUPPORT_LICENSE_KEY}")
fi

# FUNCTIONS

command_exists() {
    command -v "$@" > /dev/null 2>&1
}

is_dry_run() {
    if [ -z "$DRY_RUN" ]; then
        return 1
    else
        return 0
    fi
}

is_wsl() {
    case "$(uname -r)" in
    *microsoft* ) true ;; # WSL 2
    *Microsoft* ) true ;; # WSL 1
    * ) false;;
    esac
}

is_darwin() {
    case "$(uname -s)" in
    *darwin* ) true ;;
    *Darwin* ) true ;;
    * ) false;;
    esac
}

deprecation_notice() {
	distro=$1
	date=$2
	echo
	echo "DEPRECATION WARNING:"
	echo "    The distribution, $distro, will no longer be supported in this script as of $date."
	echo "    If you feel this is a mistake please submit an issue at https://github.com/docker/docker-install/issues/new"
	echo
	sleep 10
}

get_distribution() {
	lsb_dist=""
	# Every system that we officially support has /etc/os-release
	if [ -r /etc/os-release ]; then
		lsb_dist="$(. /etc/os-release && echo "$ID")"
	fi
	# Returning an empty string here should be alright since the
	# case statements don't act unless you provide an actual value
	echo "$lsb_dist"
}

get_distribution_version() {

	case "$1" in

		ubuntu)
			if command_exists lsb_release; then
				dist_version="$(lsb_release --release | cut -f2)"
			fi
			if [ -z "$dist_version" ] && [ -r /etc/lsb-release ]; then
				dist_version="$(. /etc/lsb-release && echo "$DISTRIB_RELEASE")"
			fi
		;;

		debian|raspbian)
			dist_version="$(sed 's/\/.*//' /etc/debian_version | sed 's/\..*//')"
			case "$dist_version" in
				10)
					dist_version_codename="buster"
				;;
				9)
					dist_version_codename="stretch"
				;;
				8)
					dist_version_codename="jessie"
				;;
			esac
		;;

		centos|rhel)
			if [ -z "$dist_version" ] && [ -r /etc/os-release ]; then
				dist_version="$(. /etc/os-release && echo "$VERSION_ID")"
			fi
		;;

		*)
			if command_exists lsb_release; then
				dist_version="$(lsb_release --release | cut -f2)"
			fi
			if [ -z "$dist_version" ] && [ -r /etc/os-release ]; then
				dist_version="$(. /etc/os-release && echo "$VERSION_ID")"
			fi
		;;

	esac

  echo "$dist_version"
}

prepare_ubuntu1804() {
  PYTHON_DEFAULT=/usr/bin/python3
  DEBIAN_FRONTEND=noninteractive
  apt-get update \
      && apt-get install -y --no-install-recommends \
         apt-utils \
         locales \
         python3-setuptools \
         python3-pip \
         software-properties-common \
         git \
      && rm -Rf /var/lib/apt/lists/* \
      && rm -Rf /usr/share/doc && rm -Rf /usr/share/man \
      && apt-get clean

  # Set Python3 to be the default (allow users to call "python" and "pip" instead of "python3" "pip3"
  update-alternatives --install /usr/bin/python python /usr/bin/python3 1

  pip3 install $pip_packages
}

prepare_centos7() {
    system_packages_pre="\
        deltarpm \
        epel-release \
        initscripts \
        git \
    "
    system_packages="python-pip"

    yum makecache fast
    yum -y install $system_packages_pre
    yum -y update
    yum -y install $system_packages
    yum clean all

    pip install $pip_packages
}

ansible_prepare_server() {
  ANSIBLE_HOME=$(dirname "$ANSIBLE_DEFAULT_HOST_LIST")
  if [[ ! -d "$ANSIBLE_HOME" ]]; then
    echo "No ansible home directory found at $ANSIBLE_HOME. Preparing..."
    mkdir --parent "$ANSIBLE_HOME"
  fi
  if [[ ! -f "$ANSIBLE_DEFAULT_HOST_LIST" ]]; then
    echo "No ansible inventory found at $ANSIBLE_DEFAULT_HOST_LIST. Preparing inventory..."
    ansible_prepare_server_inventory
  fi
  if [[ ! -f "$ANSIBLE_HOME/ansible.cfg" ]]; then
    echo "No ansible.cfg file found at $ANSIBLE_HOME/ansible.cfg. Copying ansible.default.cfg ..."
    cp "$DEVSHOP_INSTALL_PATH/ansible.cfg" "$ANSIBLE_HOME/ansible.cfg"
  fi
}

ansible_prepare_server_inventory() {
  # Strangest thing: if you leave a space after the variable "name:" the output will convert to a new line.
  IFS=$'\n'

  echo "---
# DevShop Ansible Static Inventory File
# -------------------------------------
# This Ansible Inventory file was written by devshop's install.sh script at `date`
# You may add edit this inventory file as you wish, or to additional files in /etc/ansible/group_vars or /etc/ansible/host_vars
# Run 'devshop-ansible-playbook' as root after changing this file to apply the configuration.
devshop_server:
  hosts:
    $HOSTNAME_FQDN:
  vars:" > $ANSIBLE_DEFAULT_HOST_LIST

  # Write all extra vars to the file.
  for i in ${ANSIBLE_EXTRA_VARS[@]}; do
      echo -e "    $i" >> $ANSIBLE_DEFAULT_HOST_LIST
  done
  echo $LINE
  echo "Wrote static inventory to $ANSIBLE_DEFAULT_HOST_LIST:";
  echo $LINE
  cat $ANSIBLE_DEFAULT_HOST_LIST
  echo $LINE
  echo "List Ansible Hosts:"
  ansible all --list-hosts -i $ANSIBLE_DEFAULT_HOST_LIST
}

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

do_install() {

# main

POST_INSTALL_WELCOME_MSG="

Welcome to OpenDevShop! Use the link below to sign in.

The password for user 'admin' was securely generated and hidden. 
Use 'drush @hostmaster uli' or 'devshop login' to get another login link.

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
export ANSIBLE_FORCE_COLOR=true

echo "============================================="
echo " Welcome to the DevShop Standalone Installer "
echo " Version $DEVSHOP_VERSION                    "
echo "============================================="

# Fail if not running as root (sudo)
if [ $EUID -ne 0 ]; then
    echo " This script must be run as root.  Try 'sudo -H bash install.sh'." 1>&2
    exit 1
fi

LINE=---------------------------------------------

# perform some very rudimentary platform detection
lsb_dist=$( get_distribution )
lsb_dist="$(echo "$lsb_dist" | tr '[:upper:]' '[:lower:]')"
dist_version=$( get_distribution_version $lsb_dist )

OS=$lsb_dist
VERSION=$dist_version

# Output some info.
echo " OS: $lsb_dist"
echo " Version: $dist_version"
echo " Hostname: $HOSTNAME_FQDN"
    # If /var/aegir/config/server_master/nginx.conf is found, use NGINX to install.
    # If /var/aegir/config/server_master/apache.conf is found, use apache to install.
    # This will override any selected option for web server. This is so we don't install
    # a second webserver accidentally.

# Break out preparation into separate functions.
echo $LINE
echo " Installing prerequisites (ansible, git etc)..."

case "$lsb_dist $dist_version" in
  "ubuntu 18.04")
    prepare_ubuntu1804 > /dev/null
  ;;
  "centos 7")
    prepare_centos7 > /dev/null
  ;;
  default)
    echo "Automatic ansible install is not yet supported in $lsb_dist $dist_version. Install ansible according using the instructions for your operating system found at https://docs.ansible.com/ansible/latest/installation_guide/intro_installation.html and try to run this script again."
    exit 1
  ;;
esac

if [ -f "/var/aegir/config/server_master/nginx.conf" ]; then
  SERVER_WEBSERVER=nginx
elif [ -f "/var/aegir/config/server_master/apache.conf" ]; then
  SERVER_WEBSERVER=apache
fi

# Output Web Server
echo " Web Server: $SERVER_WEBSERVER"

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

if [ -f '/root/.my.cnf' ]
then
  MYSQL_ROOT_PASSWORD=$(awk -F "=" '/pass/ {print $2}' /root/.my.cnf)
  echo " Password found at /root/.my.cnf, using $MYSQL_ROOT_PASSWORD"
else
  MYSQL_ROOT_PASSWORD=$(< /dev/urandom tr -dc _A-Z-a-z-0-9 | head -c${2:-32};echo;)
  echo " Generating new MySQL root password... $MYSQL_ROOT_PASSWORD"
  echo $MYSQL_ROOT_PASSWORD > /tmp/mysql_root_password
fi

ANSIBLE_EXTRA_VARS+=("mysql_root_password: ${MYSQL_ROOT_PASSWORD}")

# Clone the installer if $DEVSHOP_INSTALL_PATH does not exist yet.
if [ ! -d "$DEVSHOP_INSTALL_PATH" ]; then
    git clone $DOWNLOAD_URL $DEVSHOP_INSTALL_PATH
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
echo $LINE


cd $DEVSHOP_INSTALL_PATH

ansible --version
python --version

# Check that DEFAULT_HOST_LIST ansible config matches ANSIBLE_DEFAULT_HOST_LIST
if [[ `ansible-config dump | grep ${ANSIBLE_DEFAULT_HOST_LIST}` ]]; then
  echo " Ansible Inventory: $ANSIBLE_DEFAULT_HOST_LIST"
else
  echo "[ERROR] The system's ansible configuration option DEFAULT_HOST_LIST does not match the install.sh option --ansible-default-host-list ($ANSIBLE_DEFAULT_HOST_LIST)"
  echo "Result from ansible-config dump | grep $ANSIBLE_DEFAULT_HOST_LIST:"
  ansible-config dump | grep $ANSIBLE_DEFAULT_HOST_LIST
  exit 1
fi

# Prepare Ansible inventory
ansible_prepare_server

# Run the playbook.
echo $LINE
echo " Installing with Ansible..."
echo $LINE

# If ansible playbook fails syntax check, report it and exit.
ANSIBLE_PLAYBOOK="$DEVSHOP_INSTALL_PATH/$DEVSHOP_PLAYBOOK"
if [ ! `ansible-playbook --syntax-check ${ANSIBLE_PLAYBOOK}` ]; then
    echo " Ansible syntax check failed! Check ${ANSIBLE_PLAYBOOK} and try again."
    exit 1
fi

# Set the ENV vars that devshop-ansible-playbook expects, and run it.
export ANSIBLE_PLAYBOOK
export ANSIBLE_TAGS=all
export ANSIBLE_PLAYBOOK_COMMAND_OPTIONS=${ANSIBLE_PLAYBOOK_COMMAND_OPTIONS:-"--connection=local"}

# Set devshop_version at runtime so it installs the correct source code via git and we don't get the version stuck in the static inventory file.
export ANSIBLE_EXTRA_VARS="{devshop_version: ${DEVSHOP_VERSION}}"

$DEVSHOP_INSTALL_PATH/scripts/devshop-ansible-playbook

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

}

# wrapped up in a function so that we have some protection against only getting
# half the file during "curl | sh"
# shellcheck disable=SC2119
do_install