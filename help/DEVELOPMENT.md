DevShop Development
===================

This project contains a Vagrantfile for launching a devshop virtual machine.

It uses the install.sh file in this repo to provision the vagrant server.

This is the recommended install method for servers as well as vagrant boxes.

Dependencies
------------

- Drush
- Git
- Vagrant 1.5.x
- VirtualBox (VMWare has been known to have problems.  If you can solve them, please post an issue!)

Setup
-----

To get started with this project, you need to setup the right versions of it's dependencies.

Some of you may have these things, so just make sure they are up to date.

It is best to use the installers from the websites so you are sure to get the right version.


1. Install Vagrant.
  Available at [http://www.vagrantup.com/downloads.html](http://www.vagrantup.com/downloads.html).
  
  *Use version 1.5.x and up.*

2. Install VirtualBox.
  Available at [https://www.virtualbox.org/wiki/Downloads](https://www.virtualbox.org/wiki/Downloads)
  
  *Use version 4.3.x and up.*

3. Install Git.
  Available at [http://www.gitscm.com/](http://www.gitscm.com/)

  *Any version is probably fine.*

4. Install Drush.

  1. Install composer globally: https://getcomposer.org/doc/00-intro.md#globally
  2. Install drush globally: http://docs.drush.org/en/master/install/  (6.x)

5. Clone this Repo and change to it's directory.

  ```
  git clone git@github.com:opendevshop/devshop.git
  cd devshop
  ```
  
6. Edit your /etc/hosts file, adding the line:
  
  ```
  10.10.10.10  devshop.local
  ```
  
  *NOTE:* If you wish to change the IP or hostname, edit `vagrant.vars.yml` before you call vagrant up for the first time.

7. If you wish to test a "pure" devshop install, edit `vagrant.vars.yml` and set development to FALSE:

  ```yml
  # Set to TRUE if you wish to develop devshop.
  development: true
  server_hostname: devshop.local
  private_network_ip: 10.10.10.10
  install_script: install.sh
  ```

  Keeping `development: true` will clone the source code to this folder and setup synced folders to vagrant.  
  Once installed, you can edit any file in `source` to work on devshop.
  
  Setting `development: false` will provision the machine with just the install.sh script, 
  the same way you would do it on a real server.  This is useful for testing.

Usage
-----

Once that's all done you can launch and destroy the devshop VM with vagrant commands.

The first time you `vagrant up` it will install devshop. 

  ```
  vagrant up
  ```
  
Once the install script is finished you should see the "Welcome to DevShop" message with a link to login to your 
devshop front-end.
  
You can ssh into the VM (and then switch to the aegir user) with:

  ```
  $ vagrant ssh
  vagrant@devshop:~$ sudo su - aegir
  aegir@devshop:~$ 
  ```
  
If you need another login link to the front-end, simply call:

  ```
  aegir@devshop:~$ drush @hostmaster uli
  ```

DevShop Management
------------------

  Once you are the aegir user, you can interact with the devshop front-end with drush.  For example, 
  to get another login link, use the `drush uli` command:
  ```
  drush @hostmaster uli
  ```
  All server-side interactions with your sites are done as the `aegir` user.
  
  To see all hosted sites call 
  ```
  drush site-alias
  ```
  
*NOTE: When you create new projects and environments, you will need to add those URIs to your 
hosts file as well, or you will not be able to access them from your host machine.*

Repos
-----

DevShop consists of a number of code repositories.

## "DevShop": Main Project 

[github.com/opendevshop/devshop](http://github.com/opendevshop/devshop)

- DevShop install script: install.sh
- Ansible playbooks: playbook.yml, roles folder.
- Documentation
- Vagrantfile 
- build-devmaster.make file: used to build the devshop front-end.
  (Modify this file to use your fork of devmaster.)

## "DevMaster": Drupal install profile for devshop front-end

[github.com/opendevshop/devmaster](http://github.com/opendevshop/devmaster)
DevShop install script, ansible playbooks, and development tools.

## "DevShop Provision": Drush commands for devshop.

[github.com/opendevshop/devshop_provision](http://github.com/opendevshop/devshop_provision)
DevShop install script, ansible playbooks, and development tools.

Help Improve Documentation
--------------------------

Think this can be improved? You can [edit this file on GitHub](https://github.com/opendevshop/devshop/edit/0.x/README.vagrant.md) and select "Create a new branch for this commit and start a pull request.".

Thanks!
