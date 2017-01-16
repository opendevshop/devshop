DevShop Development
===================

## Docker

This project contains scripts for launching DevShop on Docker for development purposes:

  - `development-prepare.sh`: Run this script first. It will prepare the needed source code for devshop, as well as build the containers needed.
  - `development-launch.sh`: This script is run automatically by `development-prepare.sh` to launch the containers and install Devmaster.
  - `development-destroy.sh`: This script removes all traces of the devshop containers and their volumes.

## Vagrant 

The Vagrantfile in this project is now deprecated, but is still included in the `vagrant` folder if you wish to use it.

It uses the install.sh file in this repo to provision the vagrant server.

This is the recommended install method for servers as well as vagrant boxes.

See [Development with Vagrant](development-vagrant.md) for legacy instructions.

Dependencies
------------

The only tools you need on your host machine to develop devshop are:

- Git: Used to clone the source code.
- PHP-CLI: Needed to run Drush.
- Drush: Used to build the Drupal codebase on your host machine.
- Docker version 1.10.0+.
- Docker Compose version 1.6.0+.

1. [Install Git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git).

2. [Install Drush](http://docs.drush.org/en/master/install/).
2. [Install Docker](https://docs.docker.com/engine/installation/).
2. [Install Docker Compose](https://github.com/docker/compose/releases).

3. Clone this Repo and change to it's directory.

    ```
    git clone git@github.com:opendevshop/devshop.git
    cd devshop
    ```
4. Run `development-prepare.sh` script to clone the rest of the source code and prepare docker containers.

5. That's it! Look for a one-time login link that looks like:

  ```
  http://devshop.local.computer/user/reset/1/1475596064/EzLbpsTpSgKLJl7GmO0
  ```

  The `development-prepare.sh` scripts and `development-launch.sh` scripts will remain open, following the docker logs.  Press CTRL-C to cancel out of the logs if you wish.  
  
  More information on how to access the containers is output when you cancel the logs.

Repos
-----

DevShop consists of a number of code repositories.

## "DevShop": Main Project 

[github.com/opendevshop/devshop](http://github.com/opendevshop/devshop)

If you want to develop the server setup, the standalone install script, the 
documentation, or improve the Vagrantfile, fork this repo.  

*Contains:*

- DevShop install script: install.sh
- Ansible playbooks: playbook.yml, roles folder.
- Documentation
- Vagrantfile 
- build-devmaster.make file: used to build the devshop front-end.
  (Modify this file to use your fork of devmaster.)

## "DevMaster": Drupal install profile for devshop front-end

[github.com/opendevshop/devmaster](http://github.com/opendevshop/devmaster)

If you want to develop the front-end of devshop:

1. Fork this repo (https://github.com/opendevshop/devshop), and create your own branch for your feature or bugfix.
2. Edit build-devmaster.make file, and replace the devmaster url and branch
with your forked repo url and branch like so:
    
  ```
  projects[devmaster][type] = "profile"
  projects[devmaster][download][type] = "git"
  projects[devmaster][download][url] = "git@github.com:MYUSERNAME/devmaster.git"
  projects[devmaster][download][branch] = "dev-MYBRANCH"
  ```
    
See the `./source/devmaster-1.x/profiles/devmaster` folder for the fully built devmaster stack. 

Debugging
---------

The containers now contain XDEBUG that works for web requests and drush calls.

This is extremely helpful when working on tasks, which might be running in the backend.

To setup your IDE to listen for debug connections, use the following settings:

  **DGBp Proxy**:
  - idekey: PHPSTORM
  - Host: 172.17.0.1
  - Port: 9000
  
In PHPStorm, this is in the *Settings > PHP > Debug > DGBp Proxy* page.

Don't forget to "Start Listening to PHP Debug Connections", get an XDEBUG plugin for your browser and enable it for devshop.local.computer, and to set a breakpoint or an error to see the debugger work.

If using PHPStorm, It will ask you to map files. You should map ./aegir-home to /var/aegir as "Absolute path on the server".

If your Docker machine IP is not 172.17.0.1, you can change it but you must also change a line in docker-compose.yml:

        XDEBUG_CONFIG: "remote_host=172.17.0.1 idekey=PHPSTORM"


Help Improve Documentation
--------------------------

Think this can be improved? You can [edit this file on GitHub](https://github.com/opendevshop/devshop/edit/0.x/README.vagrant.md) and select "Create a new branch for this commit and start a pull request.".

Thanks!
