#!/usr/bin/env bash

# Usage:
# set-user-ids NAME UID GID
#
set -e

PREFIX='𝙋𝙍𝙊𝙑𝙄𝙎𝙄𝙊𝙉 set-user-ids.sh ║'

USER_NAME=$1
USER_UID=$2
WEB_UID=$3

echo "$PREFIX Recreating user '$USER_NAME' UID/GID to '$USER_UID'...
"
userdel $USER_NAME
chown $USER_UID:$USER_UID /var/$USER_NAME -R

addgroup --gid $USER_UID $USER_NAME
useradd --no-log-init --uid $USER_UID --gid $USER_UID --system --home-dir /var/$USER_NAME $USER_NAME

echo "$PREFIX Changing user 'www-data' UID/GID to '$WEB_UID'...
"

userdel www-data
addgroup --gid $WEB_UID www-data
useradd --no-log-init --uid $WEB_UID --gid $WEB_UID --system  www-data
