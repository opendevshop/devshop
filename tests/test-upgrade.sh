#!/bin/bash

set -ex
drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y
run-tests.sh

#
## Get argument as the version we should install.
#UPGRADE_FROM_VERSION=$1
#UPGRADE_TO_MAKEFILE=$2
#
#echo ">env"
#env
#
## If repo being tested is devshop... use the build branch as the upgrade target.
#if [ "$TRAVIS_REPO_SLUG" == "opendevshop/devshop" ]; then
#
#    echo "Repo opendevshop/devshop found..."
#
#    # If TRAVIS_PULL_REQUEST_BRANCH variable doesn't exist, it's not a pull request, use the $TRAVIS_BRANCH variable.
#    if [ -z $TRAVIS_PULL_REQUEST_BRANCH ]; then
#      UPGRADE_TO_VERSION=$TRAVIS_BRANCH
#    else
#      UPGRADE_TO_VERSION=$TRAVIS_PULL_REQUEST_BRANCH
#    fi
#
#else
#
#    echo "Repo $TRAVIS_REPO_SLUG found..."
#
#    # If DEVSHOP_UPGRADE_TO_VERSION variable is not set, use 1.x.
#    if [ -z $DEVSHOP_UPGRADE_TO_VERSION ]; then
#      UPGRADE_TO_VERSION="1.x"
#    else
#      UPGRADE_TO_VERSION=$DEVSHOP_UPGRADE_TO_VERSION
#    fi
#fi
#
#if [ -z $UPGRADE_FROM_VERSION ]; then
#  echo "No arguments found. Please specify the version to upgrade from:"
#  echo "  test-upgrade.sh 1.0.0-beta10 1.x"
#  exit 1
#elif [ -z $UPGRADE_TO_VERSION ]; then
#  echo "No target version found. Please specify the version to upgrade to:"
#  echo "  test-upgrade.sh 1.0.0-beta10 1.x"
#  exit 1
#else
#  echo "Getting install script for version $UPGRADE_FROM_VERSION to test upgrade...";
#fi
#
#echo "Running 'devshop upgrade $UPGRADE_TO_VERSION -n --makefile=$UPGRADE_TO_MAKEFILE'..."
#
#set -ev
#curl -OL "https://github.com/opendevshop/devshop/releases/download/$UPGRADE_FROM_VERSION/install.sh"
#sudo bash install.sh --makefile=https://raw.githubusercontent.com/opendevshop/devshop/$UPGRADE_FROM_VERSION/build-devmaster.make
#
#devshop self-update $UPGRADE_TO_VERSION
#devshop upgrade $UPGRADE_TO_VERSION -n --makefile=$UPGRADE_TO_MAKEFILE
#
#su - aegir -c "drush @hostmaster hosting-tasks"
#
#su - aegir -c "devshop status"
