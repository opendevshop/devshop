#!/bin/bash

AEGIR_VERSION=7.x-3.x
DEVSHOP_VERSION=1.x
echo "==========================ÆGIR=========================="
echo "------------------------ DEVSHOP------------------------"
echo " Hello there.                                           "
echo " Let's prepare a development environment for you.       "
echo "--------------------------------------------------------"

if [ ! -d aegir-home ]; then
  echo "Æ | Creating aegir-home directory..."
  mkdir aegir-home
fi

if [ ! -d provision ]; then
  echo "Æ | Cloning Provision..."
  git clone git@git.drupal.org:project/provision.git
  cd provision
  git checkout $AEGIR_VERSION
  cd ..
fi

cd aegir-home

# Build a full hostmaster frontend on the host with drush make, with working-copy option.
if [ ! -d devmaster-$DEVSHOP_VERSION ]; then
   echo "Æ | Building devmaster with drush make..."
   drush make ../build-devmaster.make devmaster-$DEVSHOP_VERSION --working-copy --no-gitinfofile
fi

# Clone drush packages.
if [ ! -d .drush ]; then
    echo "Æ | Creating .drush/commands folder..."
    mkdir -p .drush/commands
    cd .drush/commands
    echo "Æ | Cloning Registry Rebuild..."
    git clone git@git.drupal.org:project/registry_rebuild.git --branch 7.x-2.x
    cd ../..
fi

cd ../

# Clone tests
#if [ ! -d aegir-home/tests ]; then
#  echo "Æ | Cloning tests..."
#  git clone git@github.com:aegir-project/tests.git aegir-home/tests
#fi

# Clone documentation
if [ ! -d documentation ]; then
  echo "Æ | Cloning documentation..."
  git clone git@github.com:opendevshop/documentation.git
fi

# Clone dockerfiles
if [ ! -d aegir-dockerfiles ]; then
  echo "Æ | Cloning aegir-dockerfiles..."
  git clone git@github.com:aegir-project/dockerfiles.git aegir-dockerfiles
fi

# Clone dockerfiles
if [ ! -d dockerfiles ]; then
  echo "Æ | Cloning dockerfiles..."
  git clone git@github.com:opendevshop/dockerfiles.git
fi

# Make symlinks for easy access to important repos
if [ ! -L provision ]; then
  echo "Æ | Creating symlinks to all git repos..."
#  ln -s aegir-home/tests 2> /dev/null
  ln -s aegir-home/.drush/commands/provision  2> /dev/null
  ln -s aegir-home/devmaster-$DEVSHOP_VERSION/profiles/devmaster 2> /dev/null
  ln -s aegir-home/devmaster-$DEVSHOP_VERSION/profiles/devmaster/modules/aegir/hosting  2> /dev/null
fi;
echo "Æ | Codebase preparation complete."

USER_UID=`id -u`

echo "--------------------------------------------------------"
echo " About to run 'docker build' command to create a custom image for you."
echo " If you wish to abort, now is the time to hit CTRL-C "
echo ""
echo " Found UID: $USER_UID "
echo ""
echo " Waiting 5 seconds..."
echo "--------------------------------------------------------"
sleep 5

echo "Æ | Running docker build --build-arg AEGIR_UID=$USER_UID -t aegir/hostmaster:own -f aegir-dockerfiles/Dockerfile aegir-dockerfiles"
docker build --build-arg AEGIR_UID=$USER_UID -t aegir/hostmaster:own -f aegir-dockerfiles/Dockerfile aegir-dockerfiles
echo "Æ | docker build -t aegir/hostmaster:local -f aegir-dockerfiles/Dockerfile-local aegir-dockerfiles"
docker build -t aegir/hostmaster:local -f aegir-dockerfiles/Dockerfile-local aegir-dockerfiles

echo "Æ | Container preparation complete."

echo "==========================ÆGIR=========================="
echo "------------------------ DEVSHOP------------------------"
echo " Development environment is now ready to be launched! "
echo " "
echo " From now on, you can run 'development-launch.sh' to save time.  "
echo " "
echo " Waiting 5 seconds then running 'development-launch.sh' ..."
echo "--------------------------------------------------------"
sleep 5


bash development-launch.sh
