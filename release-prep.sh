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
cd source/devmaster-1.x/profiles/devmaster
git checkout -b $RELEASE_BRANCH

echo "Release Branches created.  Writing version to files..."

cd ../../../../
sed -i -e "s/1.x/$VERSION/g" install.sh
sed -i -e "s/1.x/$VERSION/g" build-devmaster.make
sed -i -e "s/1.x/$VERSION/g" source/devmaster-1.x/profiles/devmaster/VERSION.txt

echo " Files with versions have been updated: "
echo "  - build-devmaster.make"
echo "  - install.sh"
echo "  - opendevshop/devmaster/VERSION.txt"
echo ""
echo "Please create a new version of devshop_stats by pushing a tag and "
echo "visiting https://www.drupal.org/node/add/project-release/2676696"
echo "Next, make sure to add the version to opendevshop/devmaster/devmaster.make"
echo "Review and commit the changes, then run 'release.sh $VERSION' "