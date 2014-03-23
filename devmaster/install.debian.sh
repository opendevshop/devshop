#!/bin/bash
#
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
#.
#

# Fail if not running as root (sudo)
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

# Let's block interaction
export DEBIAN_FRONTEND=noninteractive

# Generate a secure password for MySQL
# Saves this password to /tmp/mysql_root_password in case you have to run the
# script again.
MYSQL_ROOT_USER=root
if [ -f '/tmp/mysql_root_password' ]
then
  MYSQL_ROOT_PASSWORD=$(cat /tmp/mysql_root_password)
  echo "Password found, using $MYSQL_ROOT_PASSWORD"
else
  MYSQL_ROOT_PASSWORD=$(< /dev/urandom tr -dc _A-Z-a-z-0-9 | head -c${1:-32};echo;)
  echo "Generating new MySQL root password... $MYSQL_ROOT_PASSWORD"
  echo $MYSQL_ROOT_PASSWORD > /tmp/mysql_root_password
fi

# Check for travis
if [ "$TRAVIS" = "true" ]; then
  echo "TRAVIS DETECTED! Setting 'travis' user password."
  MYSQL_ROOT_PASSWORD=password
  MYSQL_ROOT_USER=root
fi

# Add aegir debian sources
if [ -f '/etc/apt/sources.list.d/aegir-stable.list' ]
  then echo "Aegir apt sources found."
else
  echo "Adding Aegir apt sources."
  echo "deb http://debian.aegirproject.org stable main" | tee -a /etc/apt/sources.list.d/aegir-stable.list
  wget -q http://debian.aegirproject.org/key.asc -O- | apt-key add -
  apt-get update
fi

# Setup MySQL
if [ ! `which mysql` ] || [ "$TRAVIS" = "true" ]; then
  # Pre-set mysql root pw
  echo debconf mysql-server/root_password select $MYSQL_ROOT_PASSWORD | debconf-set-selections
  echo debconf mysql-server/root_password_again select $MYSQL_ROOT_PASSWORD | debconf-set-selections

  # Install mysql server before aegir, because we must secure it before aegir.
  apt-get install mysql-server -y

  # MySQL Secure Installation
  # Delete anonymous users
  mysql -u "$MYSQL_ROOT_USER" -p"$MYSQL_ROOT_PASSWORD" -D mysql -e "DELETE FROM user WHERE User='' OR Password='';"

  # Delete test table records
  mysql -u "$MYSQL_ROOT_USER" -p"$MYSQL_ROOT_PASSWORD" -D mysql -e "DROP DATABASE test;"
  mysql -u "$MYSQL_ROOT_USER" -p"$MYSQL_ROOT_PASSWORD" -D mysql -e "DELETE FROM mysql.db WHERE Db LIKE 'test%';"
  mysql -u "$MYSQL_ROOT_USER" -p"$MYSQL_ROOT_PASSWORD" -D mysql -e "FLUSH PRIVILEGES;"

  echo 'Secured' > /etc/mysql-secured
fi

# Check database connectivity.  Early failure == faster testing.
echo "Checking Database Access for: $MYSQL_ROOT_USER using password '$MYSQL_ROOT_PASSWORD'"
if mysql -u "$MYSQL_ROOT_USER" -p"$MYSQL_ROOT_PASSWORD" -e 'use mysql' -h 'localhost' ; then
  echo "Database Access granted for $MYSQL_ROOT_USER using password '$MYSQL_ROOT_PASSWORD'"
else
  echo "Cannot access database as $MYSQL_ROOT_USER"
  exit 1
fi

# Check anonymous mysql access
echo "Checking anonyous Database Access... This MUST result in access denied..."
if mysql -u "NotARealUser"; then
  echo "Database is accessible by 'NotARealUser'. This means the mysql installation is not secure!"
  exit 1
fi

if  [ ! `which drush` ]; then
  # Install drush
  apt-get install drush=4.5-6 -y

  # Install Provision, git, supervisor
  apt-get install aegir-provision php5 php5-gd unzip git supervisor -y

  # Using drush, install provision_git, provision_logs, provisions_tasks_extra
  # @TODO: Figure out a nicer way to setup a lot of drush projects. Is is possible with drush makefiles?
  # @TODO: Make VERSION a variable, see hostmaster-install.
  su - aegir -c "drush dl provision_git-6.x devshop_provision-6.x --destination=/var/aegir/.drush -y"
  su - aegir -c "drush dl provision_logs-6.x provision_solr-6.x provision_tasks_extra-6.x --destination=/var/aegir/.drush -y"

  # @TODO: Should we move this to top so it is "configurable"?
  MAKEFILE="/var/aegir/.drush/devshop_provision/build-devshop.make"
  COMMAND="drush devshop-install --version=6.x-1.x --aegir_db_pass=$MYSQL_ROOT_PASSWORD --aegir_db_user=$MYSQL_ROOT_USER --makefile=$MAKEFILE --profile=devshop -y"
  echo "Running...  $COMMAND"
  su - aegir -c "$COMMAND"
fi

# Adding Supervisor
if [ ! -f '/etc/supervisor/conf.d/hosting_queue_runner.conf' ]
  then
  # Following instructions from hosting_queue_runner README:
  # http://drupalcode.org/project/hosting_queue_runner.git/blob_plain/HEAD:/README.txt
  # Copy sh script and chown
  cp /var/aegir/devshop-6.x-1.x/profiles/devshop/modules/contrib/hosting_queue_runner/hosting_queue_runner.sh /var/aegir
  chown aegir:aegir /var/aegir/hosting_queue_runner.sh
  chmod 700 /var/aegir/hosting_queue_runner.sh

  # Setup config
  echo '[program:hosting_queue_runner]
; Adjust the next line to point to where you copied the script.
command=/var/aegir/hosting_queue_runner.sh
user=aegir
numprocs=1
stdout_logfile=/var/log/hosting_queue_runner
autostart=TRUE
autorestart=TRUE
; Tweak the next line to match your environment.
environment=HOME="/var/aegir",USER="aegir",DRUSH_COMMAND="/usr/bin/drush"' > /etc/supervisor/conf.d/hosting_queue_runner.conf
  service supervisor stop
  service supervisor start
fi

# Create SSH Keypair and Config
if [ ! -d '/var/aegir/.ssh' ]
  then
  su aegir -c "mkdir /var/aegir/.ssh"
  su aegir -c "ssh-keygen -t rsa -q -f /var/aegir/.ssh/id_rsa -P \"\""
  su aegir -c "drush @hostmaster --always-set --yes vset devshop_public_key \"\$(cat /var/aegir/.ssh/id_rsa.pub)\""

  # Create a ssh config file so we don't have to approve every new host.
  echo "Host *drupal.org
    StrictHostKeyChecking no
Host *github.com
    StrictHostKeyChecking no
Host *bitbucket.org
    StrictHostKeyChecking no
Host *acquia.com
    StrictHostKeyChecking no
Host *drush.in.com
    StrictHostKeyChecking no
" > /var/aegir/.ssh/config
  chown aegir:aegir /var/aegir/.ssh/config
  chmod 600 /var/aegir/.ssh/config
fi

# Detect Install, notify the user.
if [  ! -f '/var/aegir/.drush/hostmaster.alias.drushrc.php' ]; then

  echo "╔═════════════════════════════════════════════════════════════════════╗"
  echo "║ It appears something failed during installation.                    ║"
  echo "║ There is no \`/var/aegir/.drush/hostmaster.alias.drushrc.php\` file.║"
  echo "╚═════════════════════════════════════════════════════════════════════╝"
else

  echo "╔═══════════════════════════════════════════════════════════════╗"
  echo "║           ____  Welcome to  ____  _                           ║"
  echo "║          |  _ \  _____   __/ ___|| |__   ___  _ __            ║"
  echo "║          | | | |/ _ \ \ / /\___ \| '_ \ / _ \| '_ \           ║"
  echo "║          | |_| |  __/\ V /  ___) | | | | (_) | |_) |          ║"
  echo "║          |____/ \___| \_/  |____/|_| |_|\___/| .__/           ║"
  echo "║                                              |_|              ║"
  echo "╟───────────────────────────────────────────────────────────────╢"
  echo "║ If you are still having problems you may submit an issue at   ║"
  echo "║   http://drupal.org/node/add/project-issue/devshop            ║"
  echo "╟───────────────────────────────────────────────────────────────╢"
  echo "║ NOTES                                                         ║"
  echo "║ Your MySQL root password was set as $MYSQL_ROOT_PASSWORD      ║"
  echo "║ This password was saved to /tmp/mysql_root_password.          ║"
  echo "║ You might want to delete it or reboot to remove it.           ║"
  echo "║                                                               ║"
  echo "║ An SSH keypair has been created in /var/aegir/.ssh            ║"
  echo "║                                                               ║"
  echo "║  Supervisor is running Hosting Queue Runner.                  ║"
  echo "╠═══════════════════════════════════════════════════════════════╣"
  echo "║ Use this link to login:                                       ║"
  echo "╚═══════════════════════════════════════════════════════════════╝"
  echo " `su - aegir -c"drush @hostmaster uli"`    "

fi
