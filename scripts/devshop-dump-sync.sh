#!/usr/bin/env bash
set -e
SITE_ALIAS=$1
SITE_DUMP_DIR=/tmp/devshop/$SITE_ALIAS/files
SITE_DUMP_ARCHIVE=/tmp/devshop/$SITE_ALIAS/dumpdb.tar.gz
DRUSH_DUMP_COMMAND="$SITE_ALIAS dumpdb --dump-dir=$SITE_DUMP_DIR --quiet --structure-tables-key=common"

DESTINATION_SERVER=$2
DESTINATION_SITE_ALIAS=$3
COMPRESS_COMMAND="GZIP='--rsyncable' tar czf $SITE_DUMP_ARCHIVE $SITE_DUMP_DIR"
RSYNC_COMMAND="rsync -vz --progress $SITE_DUMP_ARCHIVE $DESTINATION_SERVER:$SITE_DUMP_ARCHIVE"

EXAMPLE_COMMAND="devshop-dump-sync @production aegir@devshop.cloud @mysite.production"

echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  Hi! I am the devshop-sync tool."
echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  You can run me on your server to easily sync your Drupal site's data."

echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣"

if [ -z "$SITE_ALIAS" ] || [ -z "$DESTINATION_SERVER" ] || [ -z "$DESTINATION_SITE_ALIAS" ]; then
  echo "Please specify a site alias, destination server, and destination site alias. "

  echo "ie. 'devshop-dump-sync @production aegir@devshop.cloud @mysite.production'"
  exit 1
fi

echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  This tool assumes drush and drush syncdb are installed."
echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  Checking for drush and syncdb..."

drush --version > /dev/null 2>&1
if ! drush --help syncdb > /dev/null 2>&1; then
   echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  syncdb not found. Installing..."
   drush dl syncdb-7.x > /dev/null 2>&1
fi

drush --help syncdb > /dev/null 2>&1

echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣"

echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  Drush and SyncDB tool found!"
echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  Running command: drush $DRUSH_DUMP_COMMAND..."

if `drush $DRUSH_DUMP_COMMAND`; then
  echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣"
  echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  Command run successfully. Database files are in $SITE_DUMP_DIR."
fi

echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  Compressing files: $COMPRESS_COMMAND"

if `$COMPRESS_COMMAND`; then
  echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  Files compressed successfully. Archive available at $SITE_DUMP_ARCHIVE"
fi

echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  Rsyncing database to remote server $DESTINATION_SERVER..."
echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  Running Command: $RSYNC_COMMAND";
if `$RSYNC_COMMAND`; then
  echo ""
  echo "𝗗𝗘𝗩𝗦𝗛𝗢𝗣  Files copied successfully. Archive available at $DESTINATION_SERVER:$SITE_DUMP_ARCHIVE"

fi


echo "TODO: Export the tar on the remote and run drush importdb.


