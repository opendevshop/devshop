#!/bin/bash
#
#  DevShop Standalone Install Script
#  =================================
#
#  This script will install a full devshop server from scratch.
#
#  Please read the full "Installing DevShop" instructions at https://devshop.readthedocs.org/en/latest/install/
#
#  Before you start, please visit https://github.com/opendevshop/devshop/releases to be sure you have the latest version of this script,
#  Or you may try the 0.x script with the URL https://raw.githubusercontent.com/opendevshop/devshop/0.x/install.sh
#
#  Must run with root or sudo privileges:
#
#    ubuntu@devshop:~$ wget https://raw.githubusercontent.com/opendevshop/devshop/0.x/install.sh
#    ubuntu@devshop:~$ bash install.sh
#
#  Options:
#    --hostname           The desired fully qualified domain name to set as this machine's hostname
#    --server_webserver   Set to 'nginx' if you want to use that as your webserver instead of apache.
#    --makefile           The makefile to use to build the front-end site.
#    --playbook           The Ansible playbook.yml file to use other than the included playbook.yml.

# Version used for cloning devshop playbooks
# Must be a branch or tag.
DEVSHOP_VERSION=ubuntu-16
SERVER_WEBSERVER=apache
MAKEFILE_PATH=''

echo "============================================="
echo " Welcome to the DevShop Standalone Installer "
echo "                   v $DEVSHOP_VERSION        "
echo "============================================="

# Fail if not running as root (sudo)
if [ $EUID -ne 0 ]; then
    echo " This script must be run as root.  Try 'sudo bash install.sh'." 1>&2
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

# If on travis, use localhost as the hostname
if [ "$TRAVIS" == "true" ]; then
  echo "TRAVIS DETECTED! Setting Hostname to 'localhost'."
  HOSTNAME_FQDN="localhost"
fi

LINE=---------------------------------------------

# Detect playbook path option
while [ $# -gt 0 ]; do
  case "$1" in
    --playbook=*)
      PLAYBOOK_PATH="${1#*=}"
      ;;
    --makefile=*)
      MAKEFILE_PATH="${1#*=}"
      ;;
    --server-webserver=*)
      SERVER_WEBSERVER="${1#*=}"
      ;;
    --hostname=*)
      HOSTNAME_FQDN="${1#*=}"
      ;;
    *)
      echo $LINE
      echo ' Invalid argument for --server-webserver. Must be nginx or apache.'
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

if [ $PLAYBOOK_PATH ]; then
    :
# Detect playbook next to the install script
elif [ -f "$DEVSHOP_SCRIPT_PATH/playbook.yml" ]; then
    PLAYBOOK_PATH=$DEVSHOP_SCRIPT_PATH
else
    PLAYBOOK_PATH=/usr/share/devshop
fi

echo $LINE

# Notify user we are using the found webserver.
if [ -f "/var/aegir/config/server_master/nginx.conf" ]; then
  echo " An existing Aegir NGINX installation was found. Using 'nginx' for variable 'server_webserver'"
  echo $LINE
elif [ -f "/var/aegir/config/server_master/apache.conf" ]; then
  echo " An existing Aegir Apache installation was found. Using 'apache' for variable 'server_webserver'"
  echo $LINE
fi

# Fail if server_webserver is not apache or nginx
if [ $SERVER_WEBSERVER != 'nginx' ] && [ $SERVER_WEBSERVER != 'apache' ]; then
  echo ' Invalid argument for --server-webserver. Must be nginx or apache.'
  exit 1
fi

# If ansible command is not available, install it.
# Decided on "hash" thanks to http://stackoverflow.com/questions/592620/check-if-a-program-exists-from-a-bash-script
# After testing this thoroughly on centOS and ubuntu, I think we should use command -v
if [ ! `command -v ansible` ]; then
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

        apt-get update -qq
        apt-get install $PACKAGE -y -qq
        apt-add-repository ppa:ansible/ansible -y
        apt-get update -qq
        apt-get install ansible -y -qq

    elif [ $OS == 'centos' ] || [ $OS == 'rhel' ] || [ $OS == 'redhat' ] || [ $OS == 'fedora'  ]; then

        # Build ansible from source to ensure the latest version.
        yum install -y git epel-release redhat-lsb-core > /dev/null 1>&1
        git clone http://github.com/ansible/ansible.git --recursive --branch stable-2.0

        # dir may not exist, or it may exist as a symlink.  lets handle this a little better.
        if ! [ -d "ansible" ]; then
          echo "The directory ansible does not exist which means git clone failed.  This could be a permission or link issue.  Check the referenced directory."
          exit 1
        else

          # Build ansible RPM from source code.
          yum install -y which rpm-build make asciidoc git python-setuptools python2-devel PyYAML python-httplib2 python-jinja2 python-keyczar python-paramiko python-six sshpass
          cd ansible
          git checkout v2.0.1.0-1
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

else
    echo " Ansible already installed. Skipping installation."
    echo $LINE
fi

# Install git.
if [ $OS == 'ubuntu' ] || [ $OS == 'debian' ]; then
  apt-get install git -y -qq

elif [ $OS == 'centos' ] || [ $OS == 'redhat' ] || [ $OS == 'fedora'  ]; then
    yum install epel-release -y
    yum install git -y
fi

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
#  MAKEFILE_PATH=/usr/share/devshop/build-devmaster.make
  echo $LINE
fi

# If MAKEFILE PATH is not found, default to CLI's build-devmaster.
if [ ! -f "$MAKEFILE_PATH" ]; then
  MAKEFILE_PATH=/usr/share/devshop/build-devmaster.make
  echo $LINE
fi

echo " Playbook: $PLAYBOOK_PATH/playbook.yml "
echo " Makefile: $MAKEFILE_PATH "
echo $LINE


cd $PLAYBOOK_PATH

# Create inventory file
if [ ! -f "inventory" ]; then
  echo $HOSTNAME_FQDN > inventory
  echo "Created inventory file."
else
  echo "Inventory file found."
fi

echo " Installing ansible roles..."
ansible-galaxy install -r "$PLAYBOOK_PATH/roles.yml" --force
echo $LINE

# If ansible playbook fails syntax check, report it and exit.
if [[ ! `ansible-playbook -i inventory --syntax-check playbook.yml` ]]; then
    echo " Ansible syntax check failed! Check installers/ansible/playbook.yml and try again."
    exit 1
fi

# Run the playbook.
echo " Installing with Ansible..."
echo $LINE

ANSIBLE_EXTRA_VARS="server_hostname=$HOSTNAME_FQDN mysql_root_password=$MYSQL_ROOT_PASSWORD playbook_path=$PLAYBOOK_PATH aegir_server_webserver=$SERVER_WEBSERVER devshop_version=$DEVSHOP_VERSION"

if [ "$TRAVIS" == "true" ]; then
  ANSIBLE_EXTRA_VARS="$ANSIBLE_EXTRA_VARS travis=true travis_repo_slug=$TRAVIS_REPO_SLUG travis_branch=$TRAVIS_BRANCH travis_commit=$TRAVIS_COMMIT supervisor_running=false"
else
  ANSIBLE_EXTRA_VARS="$ANSIBLE_EXTRA_VARS travis=false supervisor_running=true"
fi

if [ -n "$MAKEFILE_PATH" ]; then
  ANSIBLE_EXTRA_VARS="$ANSIBLE_EXTRA_VARS devshop_makefile=$MAKEFILE_PATH"
fi

# If testing in travis, disable supervisor.
if [ "$TRAVIS" == "true" ]; then
  ANSIBLE_EXTRA_VARS="$ANSIBLE_EXTRA_VARS supervisor_running=false"
fi

if [ -n "$ANSIBLE_EXTRA_VARS" ]; then
  ANSIBLE_EXTRA_VARS="$ANSIBLE_EXTRA_VARS devshop_makefile=$MAKEFILE_PATH"
fi

if [ $SERVER_WEBSERVER == 'apache' ]; then
  PLAYBOOK_FILE="playbook.yml"
elif [ $SERVER_WEBSERVER == 'nginx' ]; then
  PLAYBOOK_FILE="playbook-nginx.yml"
fi

ansible-playbook -i inventory $PLAYBOOK_FILE --connection=local --extra-vars "$ANSIBLE_EXTRA_VARS"

# @TODO: Remove. We should do this in the playbook, right?
# Run Composer install to enable devshop cli
#cd $PLAYBOOK_PATH
#composer install

# Run devshop status, return exit code.
su - aegir -c "devshop status"
if [ ${PIPESTATUS[0]} == 0 ]; then
  su - aegir -c "devshop login"
  echo ""
  echo "Welcome to OpenDevShop! Use the link above to login."
  echo ""
  echo "You can run the command 'devshop login' to get another login link."
  echo ""
  echo "Please visit http://getdevshop.com for help and information."
  echo ""
  echo "Join the development community at https://github.com/opendevshop/devshop"
  echo ""
  echo "Thanks!"
  echo "--The OpenDevShop Team"
  echo ""

  echo "  Issues: https://github.com/opendevshop/devshop/issues"
  echo "  Chat: https://gitter.im/opendevshop/devshop "
  echo "  Code: https://github.com/opendevshop/devshop"
  echo ""
  exit 0
else
  echo "The command 'devshop status' had an error. Check the logs and try again."
  exit 1
fi
