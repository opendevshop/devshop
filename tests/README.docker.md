DevShop on Docker
=================

We've toyed with this for a while but now it's time to get serious.

We are now using docker on Travis-ci.org.

DevShop runs much faster in docker containers than the Vagrant VM. I am going to start using for testing and development.

To launch devshop on Docker:

1. Clone this repo. cd to this folder.
2. Edit your `/etc/hosts` file so that devshop.docker points to 127.0.0.1
2. Run the launch script `$ bash docker-launch-devshop.sh`.  If you must run `sudo` to use the `docker` command, run `sudo bash docker-launch-devshop.sh`.
3. It will build the containers, run the install script, and then run the tests!
4. To "get into the server" as the aegir user:

    $ docker exec -ti devshop_server su - aegir
   
Next steps: 

  Getting a volume mounted so we can use it for development.
  Create a `.terra.yml` file that can run devshop. Patch terra to make it work if need be.