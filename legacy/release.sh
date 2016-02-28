#!/bin/bash

if [ ! $1 ]; then
    echo "You must specify a version"
    exit 1
else
    RELEASE_TAG=$1
    RELEASE_BRANCH="release-$RELEASE_TAG"
fi

if [ ! -f install.sh ]; then
  echo "install.sh not found. You must run this script in the devshop root."
  exit 1
fi

echo "Creating release tag $RELEASE_TAG in devshop..."
git tag $RELEASE_TAG

echo "Pushing release tag and branch $RELEASE_TAG in devshop..."
git push origin $RELEASE_BRANCH
git push origin $RELEASE_TAG

echo "Creating release tag $RELEASE_TAG in devmaster..."
cd source/devmaster-0.x/profiles/devmaster
git tag $RELEASE_TAG

echo "Pushing release tag and branch $RELEASE_TAG in devmaster..."
git push origin $RELEASE_BRANCH
git push origin $RELEASE_TAG

echo "Creating release tag $RELEASE_TAG in devshop_provision..."
cd ../../../drush/devshop_provision
git tag $RELEASE_TAG

echo "Pushing release tag and branch $RELEASE_TAG in devshop_provision..."
git push origin $RELEASE_BRANCH
git push origin $RELEASE_TAG

echo "Release tags created and pushed.  Please visit GitHub to edit the Tag to create a release."
