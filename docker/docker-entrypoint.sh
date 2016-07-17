#!/bin/bash

HOSTNAME=`hostname --fqdn`

# Returns true once mysql can connect.
# Thanks to http://askubuntu.com/questions/697798/shell-script-how-to-run-script-after-mysql-is-ready
mysql_ready() {
    mysqladmin ping --host=database --user=root --password=$MYSQL_ROOT_PASSWORD > /dev/null 2>&1
}

while !(mysql_ready)
do
   sleep 3
   echo "waiting for mysql ..."
done

echo "========================="
echo "Hostname: $HOSTNAME"
echo "Makefile: $AEGIR_MAKEFILE"
echo "Client Name: $AEGIR_CLIENT_NAME"
echo "Client Email: $AEGIR_CLIENT_EMAIL"

echo "-------------------------"
echo "Installing provision..."
drush dl --destination=/var/aegir/.drush provision-7 -y
drush cc drush

echo "-------------------------"
echo "Running: drush hostmaster-install"

# Exit on the first failed line.
set -e

drush hostmaster-install -y --strict=0 $HOSTNAME \
  --aegir_db_host=database \
  --aegir_db_pass=$MYSQL_ROOT_PASSWORD \
  --aegir_db_port=3306 \
  --aegir_db_user=root \
  --aegir_host=$HOSTNAME \
  --client_name=$AEGIR_CLIENT_NAME \
  --client_email=$AEGIR_CLIENT_EMAIL \
  --makefile=$AEGIR_MAKEFILE \
  --profile=$AEGIR_PROFILE \
  --version=$AEGIR_VERSION

# Output a login link. If hostmaster is already installed, `drush hostmaster-install` doesn't give us a link.
drush @hostmaster uli

# Run the hosting queue
drush @hostmaster en hosting_queued -y

drush cc drush

# Run whatever is the Docker CMD.
`$@`
