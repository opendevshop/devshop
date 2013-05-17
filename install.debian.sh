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

# Generate a secure password for MySQL
# Saves this password to /tmp/mysql_root_password in case you have to run the
# script again.
if [ -f '/tmp/mysql_root_password' ]
then
  MYSQL_ROOT_PASSWORD=$(cat /tmp/mysql_root_password)
  echo "Password found, using $MYSQL_ROOT_PASSWORD"
else
  MYSQL_ROOT_PASSWORD=$(< /dev/urandom tr -dc _A-Z-a-z-0-9 | head -c${1:-32};echo;)
  echo "Generating new MySQL root password... $MYSQL_ROOT_PASSWORD"
  echo $MYSQL_ROOT_PASSWORD > /tmp/mysql_root_password
fi

# Add aegir debian sources
if [ -f '/etc/apt/sources.list.d/aegir-stable.list' ]
  then echo "Aegir apt sources found."
else
  echo "Adding Aegir apt sources."
  echo "deb http://debian.aegirproject.org stable main" | tee -a /etc/apt/sources.list.d/aegir-stable.list
  wget -q http://debian.aegirproject.org/key.asc -O- | apt-key add -
  apt-get update

  # Pre-set mysql root pw
  echo debconf mysql-server/root_password select $MYSQL_ROOT_PASSWORD | debconf-set-selections
  echo debconf mysql-server/root_password_again select $MYSQL_ROOT_PASSWORD | debconf-set-selections

  # Install mysql server before aegir, because we must secure it before aegir.
  apt-get install mysql-server -y

  # MySQL Secure Installtion
  # Delete anonymous users
  mysql -u root -p"$MYSQL_ROOT_PASSWORD" -D mysql -e "DELETE FROM user WHERE User='';"

  # Delete test table records
  mysql -u root -p"$MYSQL_ROOT_PASSWORD" -D mysql -e "DROP DATABASE test;"
  mysql -u root -p"$MYSQL_ROOT_PASSWORD" -D mysql -e "DELETE FROM mysql.db WHERE Db LIKE 'test%';"
  mysql -u root -p"$MYSQL_ROOT_PASSWORD" -D mysql -e "FLUSH PRIVILEGES;"

  echo 'Secured' > /etc/mysql-secured

  # Install aegir-provision and other tools
  # @TODO: Preseed postfix settings
  apt-get install aegir-provision php5 php5-gd unzip git supervisor -y
fi

# Download DevShop backend projects (devshop_provision and provision_git)
if [ ! -d '/var/aegir/.drush/provision_git' ]
  then
  su - aegir -c "drush dl provision_git-6.x devshop_provision-6.x --destination=/var/aegir/.drush -y"
fi

# Install DevShop with drush devshop-install
if [ ! -d '/var/aegir/devshop-6.x-1.x/' ]
  then
  MAKEFILE="/var/aegir/.drush/devshop_provision/build-devshop.make"
  COMMAND="drush devshop-install --version=6.x-1.x --aegir_db_pass=$MYSQL_ROOT_PASSWORD --makefile=$MAKEFILE --profile=devshop -y"
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
  echo "[program:hosting_queue_runner]
; Adjust the next line to point to where you copied the script.
command=/var/aegir/hosting_queue_runner.sh
user=aegir
numprocs=1
stdout_logfile=/var/log/hosting_queue_runner
autostart=TRUE
autorestart=TRUE
; Tweak the next line to match your environment.
environment=HOME=\"/var/aegir\",USER=\"aegir\",DRUSH_COMMAND=\"/usr/bin/drush\"" > /etc/supervisor/conf.d/hosting_queue_runner.conf
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
  echo "StrictHostKeyChecking no" > /var/aegir/.ssh/config
  chown aegir:aegir /var/aegir/.ssh/config
  chmod 600 /var/aegir/.ssh/config
fi

# @TODO Find out best way to detect proper installation
  echo "=============================================================================="
  echo "Your MySQL root password was set as $MYSQL_ROOT_PASSWORD"
  echo "This password was saved to /tmp/mysql_root_password"
  echo "You might want to delete it or reboot so that it will be removed."
  echo ""
  echo "An SSH keypair has been created in /var/aegir/.ssh"
  echo ""
  echo "Supervisor is running Hosting Queue Runner."
  echo ""
  echo "=============================================================================="
  echo "Welcome to DevShop!"
  echo "Use the link above to login to your DevShop"
  echo "You should now reboot your server."
  echo "=============================================================================="


  # echo "============================================================="
  # echo "  DevShop was NOT installed properly!"
  # echo "  Please Review the logs and try again."
  # echo ""
  # echo "  If you are still having problems you may submit an issue at"
  # echo "  http://drupal.org/node/add/project-issue/devshop"
  # echo "============================================================="
