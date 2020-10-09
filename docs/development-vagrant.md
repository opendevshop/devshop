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

4. Install Drush (on your computer. Drush is used to build the source code.)

  1. Install composer globally: https://getcomposer.org/doc/00-intro.md#globally
  2. Install drush globally: http://docs.drush.org/en/master/install/  (6.x)

5. Clone this Repo and change to it's directory.

  ```
  git clone git@github.com:opendevshop/devshop.git
  cd devshop
  ```

6. Before you vagrant up the first time, if you wish to develop devshop:

  This repo is used for developing devshop.  To enable "development mode", create a file in this repo called 
  `.development_mode`.
  
  This file will be ignored by git, but will tell the Vagrant box to clone the devshop source code and map the folders
  to your vagrant box.
  
  The old method of enabling development mode will still work, but you must be careful not to commit the change to vars.yml:

  ```yml
  # VAGRANT variables.
  # Set development to TRUE if you wish to contribute to devshop development.
  vagrant_development: TRUE
  vagrant_private_network_ip: 10.10.10.10
  vagrant_install_script: install.sh
  ```

## Vagrant Up

If "development mode" is on (meaning, there is a file called .development_mode in the repo root), the next time you run 
the command `vagrant` it will prepare the source code for devshop in the `./source` folder.
   
  - The "front-end code" is located at `./source/devmaster-0.x`, and is mapped to `/var/aegir/devmaster-0.x` in the
   vagrant box. The actual git repo for devmaster is at `./source/devmaster-0.x/profiles/devmaster`.
  - The drush commands are all cloned to `./source/drush`, and is mapped to `/var/aegir/.drush/commands` in the vagrant box.
  - The project folder is mapped to `/vagrant` in the box, as usual.
  
  Once installed, you can edit any file in `./source` to work on devshop.
  
If `vagrant_development` is `FALSE` (the default), `vagrant up` will provision the machine with just the install.sh script, 
  the same way you would do it on a real server.  This is useful for testing and demonstration purposes.

## Vagrant Provision
  
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
  
  or, the run new devshop CLI command "login":

  ```
  aegir@devshop:~$ devshop login
  ```
  
## devshop.site

The domain name `devshop.site` is available, pointing at IP 10.10.10.10.  As long as you didn't change it in vars.yml,
you can use http://devshop.site for your devmaster front-end, and http://*.devshop.site for your hosted sites.

This means no more fiddling with your `/etc/hosts` file. All sites created in your vagrant box will be available out of 
the box.

DevShop Management
------------------

  Once you are the aegir user, you can interact with the devshop front-end with drush.

  All server-side interactions with your sites are done as the `aegir` user.  Do not run as root unless you need to 
  install extra packages or have special configuration.
  
  To see all hosted sites call 
  ```
  drush site-alias
  ```
  
DevShop.site
------------

ThinkDrop has purchased the "devshop.site" domain to use for local development of devshop.

The default hostname of the server is devshop.site, and all sites created on devshop will be at subdomains like 
http://environment.project.devshop.site


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
  3. Vagrant up!
    The `build-devmaster.make` file will be used to build the full drupal 
    distribution when you `vagrant up` for the first time.   
    
    See the `./source/devmaster-6.x-1.x/profiles/devmaster` folder for the clone
    of your repo. 

*Contains:*

DevShop install script, ansible playbooks, and development tools.

## "DevShop Provision": Drush commands for devshop.

[github.com/opendevshop/devshop_provision](http://github.com/opendevshop/devshop_provision)
DevShop install script, ansible playbooks, and development tools.

Help Improve Documentation
--------------------------

Think this can be improved? You can [edit this file on GitHub](https://github.com/opendevshop/devshop/edit/0.x/README.vagrant.md) and select "Create a new branch for this commit and start a pull request.".

Thanks!
