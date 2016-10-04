#!/bin/bash

AEGIR_VERSION=7.x-3.x
DEVSHOP_VERSION=1.x

if [ ! -d aegir-home ]; then
  echo "aegir-home directory not found. Run 'development-prepare.sh'"
  exit 1
fi

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
docker-compose logs -f

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
echo "    docker-compose exec devmaster bash            "
echo "                                                   "
echo " Then, you can run drush directly.                 "
echo "    drush @hostmaster uli                          "
echo "                                                   "
echo " To run drush from the host using docker:          "
echo "    docker-compose exec devmaster drush @hostmaster uli  "
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
