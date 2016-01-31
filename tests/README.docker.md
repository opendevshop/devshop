DevShop on Docker
=================

We've toyed with this for a while but now it's time to get serious.

We are now using docker on Travis-ci.org.

DevShop runs much faster in docker containers than the Vagrant VM. I am going to start using for testing and development.

## Helper Scripts

I've written some helper scripts that should help:

docker-launch-devshop.sh: 
will launch a devshop server inside a docker container

docker-test-devshop.sh:
Runs `devshop test` inside the devshop docker container.

docker-destroy-devshop.sh:
Kills and removes the containers, to clean up your system.

## Launching devshop on docker

To launch devshop on Docker:

1. Clone this repo. cd to this folder.
2. Edit your `/etc/hosts` file so that devshop.docker points to 127.0.0.1
2. Run the launch script `$ bash docker-launch-devshop.sh`.  If you must run `sudo` to use the `docker` command, run `sudo bash docker-launch-devshop.sh`.
3. It will build the containers, run the install script, and then run the tests!
4. To "get into the server" as the aegir user:

    $ docker exec -ti devshop_server su - aegir
   
Then you can run the devshop or drush commands:

    $ devshop login
    
## Notes
   
   This is just an experiment at this point, but the same setup is used on every code push on travis-ci.org.
   
## Next steps: 

  Getting a volume mounted so we can use it for development.
  Create a `.terra.yml` file that can run devshop. Patch terra to make it work if need be.