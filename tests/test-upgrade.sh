#!/bin/bash

set -e

# Fail if not running as root (sudo)
if [ $EUID -ne 0 ]; then
    echo " This script must be run as root.  Try 'sudo bash test-upgrade.sh 1.0.0-beta10'." 1>&2
    exit 1
fi

# Get argument as the version we should install.
UPGRADE_FROM_VERSION=$1
UPGRADE_TO_MAKEFILE=$2

echo ">env"
env

# If repo being tested is devshop... use the build branch as the upgrade target.
if [ "$TRAVIS_REPO_SLUG"=="opendevshop/devshop" ]; then

    # If TRAVIS_PULL_REQUEST_BRANCH variable doesn't exist, it's not a pull request, use the $TRAV)S_BRANCH variable.
    if [ -z $TRAVIS_PULL_REQUEST_BRANCH ]; then
      UPGRADE_TO_VERSION=$TRAVIS_BRANCH
    else
      UPGRADE_TO_VERSION=$TRAVIS_PULL_REQUEST_BRANCH
    fi

elif [ "$TRAVIS_REPO_SLUG"=="opendevshop/devmaster" ]; then

    # If DEVSHOP_UPGRADE_TO_VERSION variable is not set, use 1.x.
    if [ -z $DEVSHOP_UPGRADE_TO_VERSION ]; then
      UPGRADE_TO_VERSION="1.x"
    else
      UPGRADE_TO_VERSION=$DEVSHOP_UPGRADE_TO_VERSION
    fi
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
curl -OL "https://github.com/opendevshop/devshop/releases/download/$UPGRADE_FROM_VERSION/install.sh"
sudo bash install.sh --makefile=https://raw.githubusercontent.com/opendevshop/devshop/$UPGRADE_FROM_VERSION/build-devmaster.make

echo "Running devshop upgrade $UPGRADE_TO_VERSION command..."
devshop self-update $UPGRADE_TO_VERSION
devshop upgrade $UPGRADE_TO_VERSION -n --makefile=$UPGRADE_TO_MAKEFILE

su - aegir -c "drush @hostmaster hosting-tasks"

su - aegir -c "devshop status"
