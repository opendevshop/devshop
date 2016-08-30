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
    echo "Æ | Cloning Provision..."
    git clone git@git.drupal.org:project/provision.git
    cd provision
    git checkout $AEGIR_VERSION

    cd ..
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
echo "==========================ÆGIR=========================="
echo "------------------------ DEVSHOP------------------------"
echo "Codebase preparation complete."

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

echo "Æ | Running docker build --build-arg AEGIR_UID=$USER_UID -t aegir/hostmaster:local -f dockerfiles/Dockerfile-local dockerfiles"
docker build --build-arg AEGIR_UID=$USER_UID -t aegir/hostmaster:local -f dockerfiles/Dockerfile-local dockerfiles

echo "==========================ÆGIR=========================="
echo "------------------------ DEVSHOP------------------------"
echo " About to run 'docker compose up -d && docker-logs -f'"
echo " If you wish to abort, now is the time to hit CTRL-C "
echo ""
echo " To cancel following docker logs, hit CTRL-C. The containers will still run."
echo ""
echo " Waiting 5 seconds..."
sleep 5

docker-compose up -d
docker-compose logs -ft

echo "==========================ÆGIR=========================="
echo "------------------------ DEVSHOP------------------------"
echo " Stopped following logs. To view logs again:     "
echo "    docker-compose logs -f                       "
echo "                                                 "
echo " To stop the containers:                         "
echo "    docker-compose kill                          "
echo "                                                 "
echo " To start the same containers again and watch the logs, run: "
echo "    docker-compose up -d ; docker-compose logs -f "
echo "                                                 "
echo " To fully destroy the containers and volumes, run:"
echo "    docker-compose rm -v"
echo "                                                 "
echo " If you destroy the containers, in order to start"
echo " again, you will have to  delete the "
echo " sites/devshop.local.computer folder:   "
echo "    rm -rf aegir-home/devmaster-1.x/sites/devshop.local.computer"
echo "                                                 "
echo " To 'log in' to the container, run 'bash' with docker-compose exec.: "
echo "    docker-compose exec hostmaster bash            "
echo "                                                   "
echo " Then, you can run drush directly.                 "
echo "    drush @hostmaster uli                          "
echo "                                                   "
echo " To run drush from the host using docker:          "
echo "    docker-compose exec hostmaster drush @hostmaster uli  "
echo "                                                   "
#echo "-----------------------------------------------------"
#echo " Testing                                           "
#echo " We have behat tests you can run from inside the container:"
#echo "    docker-compose exec hostmaster bash           "
#echo "    cd tests                                     "
#echo "    composer install                               "
#echo "    bin/behat                               "
#echo ""
#echo " You can edit the tests that run in 'tests/features'."
echo "-----------------------------------------------------"
echo " Thanks! Please report any issues to http://github.com/opendevshop/devshop/issues"
echo "==========================ÆGIR=========================="
echo "------------------------ DEVSHOP------------------------"
