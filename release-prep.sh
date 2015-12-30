#!/bin/bash

if [ ! $1 ]; then
    echo "You must specify a version"
    exit 1
else
    VERSION=$1
fi

if [ ! -f install.sh ]; then
  echo "install.sh not found. You must run this script in the devshop root."
  exit 1
fi

RELEASE_BRANCH="release-$VERSION"

echo "Creating release branch $RELEASE_BRANCH in devshop..."
git checkout -b $RELEASE_BRANCH

echo "Creating release branch $RELEASE_BRANCH in devmaster..."
cd source/devmaster-0.x/profiles/devmaster
git checkout -b $RELEASE_BRANCH

echo "Creating release branch $RELEASE_BRANCH in devshop_provision..."
cd ../../../drush/devshop_provision
git checkout -b $RELEASE_BRANCH

echo "Release Branches created.  Bump your versions then run bash release.sh"
