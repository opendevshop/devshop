# Developing DevShop

DevShop is a big project with a lot of git repositories, so we've made sure to include tools to make it easier.

## Docker via Robo

We've implemented a RoboFile to allow us to use the Robo CLI to manage the development environment.

Visit [http://robo.li](http://robo.li) for more information on Robo.

## Dependencies

The only tools you need on your host machine to develop devshop are:

* Git: Used to clone the source code.
* PHP-CLI: Needed to run Drush.
* Drush: Used to build the Drupal codebase on your host machine.
* Docker version 1.10.0+.
* Docker Compose version 1.6.0+.
* [Install Git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git).
* [Install Docker](https://docs.docker.com/engine/installation/).
* [Install Docker Compose](https://github.com/docker/compose/releases).
* Clone this Repo and change to it's directory.

  ```text
   git clone git@github.com:opendevshop/devshop.git
   cd devshop
   composer install
   bin/robo up
  ```

That's it! Look for a one-time login link that looks like:

   ```text
   http://devshop.local.computer/user/reset/1/1475596064/EzLbpsTpSgKLJl7GmO0
   ```

   The `docker logs` will remain open. Press CTRL-C to cancel out of the logs if you wish, the containers will still run.

## Robo

Our Robofile.php has all the commands you need to manage a local development copy of DevShop:.

Once the robo CLI is installed, cd to the DevShop repo directory and run `robo` to see a list of available commands.

```text
Available commands:
  destroy             Destroy all containers, docker volumes, and aegir configuration.
  help                Displays help for a command
  launch              Launch devshop after running prep:host and prep:source. Use --build to build new local containers.
  list                Lists commands
  logs                Stream logs from the containers using docker-compose logs -f
  login               Get a one-time login link to Devamster.
  shell               Enter a bash shell in the devmaster container.
  stop                Stop devshop containers using docker-compose stop
  test                Run all devshop tests on the containers.
  up                  Launch devshop containers using docker-compose up and follow logs.
  build               Build devshop containers from the Dockerfiles. 
 prepare
  prepare:host        Check for docker, docker-compose and drush. Install them if they are missing.
  prepare:sourcecode  Clone all needed source code and build devmaster from the makefile.
```

## Repos

DevShop consists of a number of code repositories.

## "DevShop": Main Project

[github.com/opendevshop/devshop](http://github.com/opendevshop/devshop)

If you want to develop the server setup, the standalone install script, the documentation, or improve the Vagrantfile, fork this repo.

_Contains:_

* DevShop install script: install.sh
* Ansible playbooks: playbook.yml, roles folder.
* Documentation
* Vagrantfile 
* build-devmaster.make file: used to build the devshop front-end.

  \(Modify this file to use your fork of devmaster.\)

## "DevMaster": Drupal install profile for devshop front-end

[github.com/opendevshop/devmaster](http://github.com/opendevshop/devmaster)

If you want to develop the front-end of devshop:

1. Fork this repo \([https://github.com/opendevshop/devshop](https://github.com/opendevshop/devshop)\), and create your own branch for your feature or bugfix.
2. Edit build-devmaster.make file, and replace the devmaster url and branch with your forked repo url and branch like so:

   ```text
   projects[devmaster][type] = "profile"
   projects[devmaster][download][type] = "git"
   projects[devmaster][download][url] = "git@github.com:MYUSERNAME/devmaster.git"
   projects[devmaster][download][branch] = "dev-MYBRANCH"
   ```

See the `./source/devmaster-1.x/profiles/devmaster` folder for the fully built devmaster stack.

## Debugging

The containers now contain XDEBUG that works for web requests and drush calls.

This is extremely helpful when working on tasks, which might be running in the backend.

To setup your IDE to listen for debug connections, use the following settings:

**DGBp Proxy**:

* idekey: PHPSTORM
* Host: 172.17.0.1
* Port: 9000

In PHPStorm, this is in the _Settings &gt; PHP &gt; Debug &gt; DGBp Proxy_ page.

Don't forget to "Start Listening to PHP Debug Connections", get an XDEBUG plugin for your browser and enable it for devshop.local.computer, and to set a breakpoint or an error to see the debugger work.

If using PHPStorm, It will ask you to map files. You should map ./aegir-home to /var/aegir as "Absolute path on the server".

If your Docker machine IP is not 172.17.0.1, you can change it but you must also change a line in docker-compose.yml:

```text
    XDEBUG_CONFIG: "remote_host=172.17.0.1 idekey=PHPSTORM"
```

To detect what your machine's host IP is, you can run the following command from within the container:

```text
    docker-compose exec devmaster bash
    /sbin/ip route|awk '/default/ { print $3 }'
```

Credit to [http://stackoverflow.com/questions/22944631/how-to-get-the-ip-address-of-the-docker-host-from-inside-a-docker-container](http://stackoverflow.com/questions/22944631/how-to-get-the-ip-address-of-the-docker-host-from-inside-a-docker-container)

## Vagrant

The Vagrantfile in this project is now deprecated, but is still included in the `vagrant` folder if you wish to use it.

It uses the install.sh file in this repo to provision the vagrant server.

This is the recommended install method for servers as well as vagrant boxes.

See [Development with Vagrant](https://github.com/opendevshop/documentation/tree/4c1866b89e87467c5d6bad83343cb3e8de6230a5/development-vagrant.md) for legacy instructions.

## Help Improve Documentation

Think this can be improved? You can [edit this file on GitHub](https://github.com/opendevshop/devshop/edit/0.x/README.vagrant.md) and select "Create a new branch for this commit and start a pull request.".

Thanks!

