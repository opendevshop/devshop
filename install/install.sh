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

# Version to install (branch or tag). Must point to SCRIPT_COMMIT_SHA
# If testing a branch is needed, set the DEVSHOP_VERSION environment variable in
# the command line environment:
#
#     $ export DEVSHOP_VERSION=bug/XXX/fix
#     $ bash install.sh
#
DEVSHOP_VERSION=${DEVSHOP_VERSION:-1.x}

# Git repo to install.
DEFAULT_DOWNLOAD_URL="http://github.com/opendevshop/devshop.git"
if [ -z "$DOWNLOAD_URL" ]; then
    DOWNLOAD_URL=$DEFAULT_DOWNLOAD_URL
fi

# Parse Options
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

DEVSHOP_INSTALL_PATH=/usr/share/devshop
DEVSHOP_PLAYBOOK='roles/devshop.server/play.yml'
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

LINE=---------------------------------------------

# perform some very rudimentary platform detection
lsb_dist=$( get_distribution )
lsb_dist="$(echo "$lsb_dist" | tr '[:upper:]' '[:lower:]')"
dist_version=$( get_distribution_version $lsb_dist )

OS=$lsb_dist
VERSION=$dist_version
HOSTNAME_FQDN=`hostname --fqdn`

# Output some info.
echo " OS: $lsb_dist"
echo " Version: $dist_version"
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

# INSTALL ANSIBLE, quietly
if ! command_exists ansible; then
  echo " Installing Ansible with ./scripts/devshop-ansible-install script..."
  bash scripts/devshop-ansible-install > /dev/null
  echo " Done!"
  echo $LINE
fi

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

# Check inventory file for [devmaster] group or is executable, leave it alone.
if [[ -x "$ANSIBLE_DEFAULT_HOST_LIST" ]] || [ `cat ${ANSIBLE_DEFAULT_HOST_LIST} | grep ${HOSTNAME_FQDN}` ]; then
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
# @TODO: Remove once we know #627 is passing: Ansible roles now detect the hostname.
# ANSIBLE_EXTRA_VARS+=("server_hostname: ${HOSTNAME_FQDN}")
ANSIBLE_EXTRA_VARS+=("devshop_cli_path: ${DEVSHOP_INSTALL_PATH}")
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
echo "Wrote group variables file for devmaster to $ANSIBLE_VARS_GROUP_PATH:"
cat $ANSIBLE_VARS_GROUP_PATH
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
ANSIBLE_PLAYBOOK="$DEVSHOP_INSTALL_PATH/$DEVSHOP_PLAYBOOK"
if [ ! `ansible-playbook --syntax-check ${ANSIBLE_PLAYBOOK}` ]; then
    echo " Ansible syntax check failed! Check ${ANSIBLE_PLAYBOOK} and try again."
    exit 1
fi

export ANSIBLE_PLAYBOOK
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