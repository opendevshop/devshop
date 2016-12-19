#!/bin/bash

set -e

# Fail if not running as root (sudo)
if [ $EUID -ne 0 ]; then
    echo " This script must be run as root.  Try 'sudo bash test-upgrade.sh 1.0.0-beta10'." 1>&2
    exit 1
fi

# Get argument as the version we should install.
UPGRADE_FROM_VERSION=$1

# Detect version to install from Travis variables.
if [ -z $TRAVIS_PULL_REQUEST_BRANCH ]; then
  UPGRADE_TO_VERSION=$TRAVIS_BRANCH
else
  UPGRADE_TO_VERSION=$TRAVIS_PULL_REQUEST_BRANCH
fi

if [ -z $UPGRADE_FROM_VERSION ]; then
  echo "No arguments found. Please specify the version to upgrade from:"
  echo "  test-upgrade.sh 1.0.0-beta10 1.x"
  exit 1
elif [ -z $UPGRADE_TO_VERSION ]; then
  echo "No target version found. Please specify the version to upgrade to:"
  echo "  test-upgrade.sh 1.0.0-beta10 1.x"
  exit 1
else
  echo "Getting install script for version $UPGRADE_FROM_VERSION to test upgrade...";
fi

set -ev
wget "https://github.com/opendevshop/devshop/releases/download/$UPGRADE_FROM_VERSION/install.sh"
bash install.sh

echo "Running devshop:upgrade command..."
devshop self-update
devshop upgrade $UPGRADE_TO_VERSION -n

su - aegir -c "drush @hostmaster hosting-tasks"

su - aegir -c "devshop status"
