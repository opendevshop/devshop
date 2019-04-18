#!/bin/bash

#set -e

HOSTNAME=`hostname --fqdn`
echo "   ____              ____  _                      "
echo "  |  _ \  _____   __/ ___|| |__   ___  _ __       "
echo "  | | | |/ _ \ \ / /\___ \| '_ \ / _ \| '_ \      "
echo "  | |_| |  __/\ V /  ___) | | | | (_) | |_) |     "
echo "  |____/ \___| \_/  |____/|_| |_|\___/| .__/      "
echo "           http://getdevshop.com      |_|         "
echo "__________________________________________________"
echo ""
echo "              Docker Entrypoint                   "
echo "__________________________________________________"
echo 'DevShop | Welcome to DevShop'
echo 'DevShop | When the database is ready, we will install DevShop with the following options:'
echo "DevShop | -------------------------"
echo "DevShop | Hostname: $HOSTNAME"
echo "DevShop | Version: $AEGIR_VERSION"
echo "DevShop | Provision Version: $PROVISION_VERSION"
echo "DevShop | Database Host: $AEGIR_DATABASE_SERVER"
echo "DevShop | Makefile: $AEGIR_MAKEFILE"
echo "DevShop | Profile: $AEGIR_PROFILE"
echo "DevShop | Root: $AEGIR_HOSTMASTER_ROOT"
echo "DevShop | Client Name: $AEGIR_CLIENT_NAME"
echo "DevShop | Client Email: $AEGIR_CLIENT_EMAIL"
echo "DevShop | Working Copy: $AEGIR_WORKING_COPY"
echo "DevShop | -------------------------"
echo "DevShop | TIP: To receive an email when the container is ready, add the AEGIR_CLIENT_EMAIL environment variable to your docker-compose.yml file."
echo "DevShop | -------------------------"
echo 'DevShop | Checking /var/aegir...'
ls -lah /var/aegir
echo "DevShop | -------------------------"
echo 'DevShop | Checking /var/aegir/.drush/...'
ls -lah /var/aegir/.drush
echo "DevShop | -------------------------"
echo 'DevShop | Checking drush status...'
drush status
echo "DevShop | -------------------------"
echo "DevShop | Starting apache..."

sudo apache2ctl start

# Use drush help to determnine if Provision is installed anywhere on the system.
drush help provision-save > /dev/null 2>&1
if [ ${PIPESTATUS[0]} == 0 ]; then
    echo "DevShop | Provision Commands found."
else
    echo "DevShop | Provision Commands not found! Installing..."
    drush dl provision-$PROVISION_VERSION --destination=/var/aegir/.drush/commands -y
fi

echo "DevShop | -------------------------"
echo "DevShop | Starting services..."
sudo services-start

# Returns true once mysql can connect.
# Thanks to http://askubuntu.com/questions/697798/shell-script-how-to-run-script-after-mysql-is-ready

mysql_ready() {
    mysqladmin ping --host=$AEGIR_DATABASE_SERVER --user=root --password=$MYSQL_ROOT_PASSWORD > /dev/null 2>&1
}

# Returns true if mysql can connect to root with an empty password.
mysql_server_password_is_blank() {
    mysqladmin ping --password='' > /dev/null 2>&1
}

# If the mysql password is blank, then this is a brand new container.
#if [ mysql_server_password_is_blank ]; then

  # If the $MYSQL_ROOT_PASSWORD variable is blank, we need to either generate or lookup the password.
  if [ -z "$MYSQL_ROOT_PASSWORD" ]; then
    echo "DevShop | MYSQL_ROOT_PASSWORD variable is blank..."

    if [ -f '/var/aegir/.my.cnf' ]
    then
      MYSQL_ROOT_PASSWORD=$(awk -F "=" '/pass/ {print $2}' /var/aegir/.my.cnf)
      echo "DevShop | Password found at /var/aegir/.my.cnf, using  $MYSQL_ROOT_PASSWORD"
    else
        export MYSQL_ROOT_PASSWORD="$(pwgen -1 32)"
        echo "DevShop | Generated new root password: $MYSQL_ROOT_PASSWORD"

        echo "DevShop | Setting root password..."
        mysql --host=$AEGIR_DATABASE_SERVER --user=root --password='' -e "ALTER USER 'root'@'localhost' IDENTIFIED BY '$MYSQL_ROOT_PASSWORD'"
    fi
  fi
#fi

# Always write a .my.cnf, even if someone defines a root password.
echo "DevShop | Writing /var/aegir/.my.cnf..."
echo "[client]" > /var/aegir/.my.cnf
echo "user=root" >> /var/aegir/.my.cnf
echo "password=$MYSQL_ROOT_PASSWORD" >> /var/aegir/.my.cnf
echo "host=$AEGIR_DATABASE_SERVER" >> /var/aegir/.my.cnf
cat /var/aegir/.my.cnf

while !(mysql_ready)
do
   sleep 3
   echo "DevShop | Waiting for database host '$AEGIR_DATABASE_SERVER' ..."
done

echo "DevShop | Database active! Checking for Hostmaster Install..."

# Check if @hostmaster is already set and accessible.
drush @hostmaster vget site_name > /dev/null 2>&1
if [ ${PIPESTATUS[0]} == 0 ]; then
  echo "DevShop | Hostmaster site found... Checking for upgrade platform..."

  # Only upgrade if site not found in current containers platform.
  if [ ! -d "$AEGIR_HOSTMASTER_ROOT/sites/$HOSTNAME" ]; then
      echo "DevShop | Site not found at $AEGIR_HOSTMASTER_ROOT/sites/$HOSTNAME, upgrading!"
      echo "DevShop | Running 'drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT -y'...!"
      drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT -y
  else
      echo "DevShop | Site found at $AEGIR_HOSTMASTER_ROOT/sites/$HOSTNAME"
  fi

# if @hostmaster is not accessible, install it.
else
  echo "DevShop | Hostmaster not found. Continuing with install!"

  echo "DevShop | -------------------------"
  echo "DevShop | Running: drush cc drush"
  drush cc drush

  echo "DevShop | -------------------------"
  echo "DevShop | Running: drush hostmaster-install"

  set -ex
  drush hostmaster-install -y --strict=0 $HOSTNAME \
    --aegir_db_host=$AEGIR_DATABASE_SERVER \
    --aegir_db_pass=$MYSQL_ROOT_PASSWORD \
    --aegir_db_port=3306 \
    --aegir_db_user=root \
    --aegir_db_grant_all_hosts=1 \
    --aegir_host=$HOSTNAME \
    --client_name=$AEGIR_CLIENT_NAME \
    --client_email=$AEGIR_CLIENT_EMAIL \
    --makefile=$AEGIR_MAKEFILE \
    --profile=$AEGIR_PROFILE \
    --root=$AEGIR_HOSTMASTER_ROOT \
    --working-copy=$AEGIR_WORKING_COPY

fi

sleep 3


# Exit on the first failed line.
set -e

echo "DevShop | Running 'drush cc drush' ... "
drush cc drush

echo "DevShop | Enabling hosting queued..."
drush @hostmaster en hosting_queued -y

ls -lah /var/aegir

# We need a ULI here because aegir only outputs one on install, not on subsequent verify.
echo "DevShop | Getting a new login link ... "
drush @hostmaster uli

echo "DevShop | Clear Hostmaster caches ... "
drush cc drush
drush @hostmaster cc all

# Run whatever is the Docker CMD, typically drush @hostmaster hosting-queued
echo "DevShop | Running Docker Command '$@' ..."

exec "$@"
