DevShop
=======

Welcome to the DevShop Source code.

[![Build Status](https://travis-ci.org/opendevshop/devshop.svg?branch=0.x)](https://travis-ci.org/opendevshop/devshop)

Contents
--------

This project contains four important components to DevShop

1. install.sh
  A bash script to go from zero to DevShop.  All you need is a new server and this script.
  This script is designed to work all by itself, and is not dependent on the other files in this repo to run.

2. build-devshop.make
  The makefile used to build the DevShop front-end.

3. devshop.profile
  The Drupal installation profile for DevShop.  Used to setup the front end.
  
4. Vagrantfile
  Allows devshop to be launched with Vagrant. Used for testing and development. See README.vagrant.md for more info.

Installation
------------

1. Pick a domain and server name to use for DevShop, for example "devshop.thinkdrop.net"
2. Fire up a linux server somewhere, using that domain name as the server's hostname. (Ubuntu 12.04 is the most tested.)
  - Rackspace and DigitalOcean use the name of the server to automatically set the hostname, so use your domain name 
    as the server name when creating it.
  - On Amazon Web Services you must [change the hostname manually](http://docs.aws.amazon.com/AWSEC2/latest/UserGuide/set-hostname.html).
  - On Linode, you must also [set the hostname manually](https://www.linode.com/docs/getting-started#setting-the-hostname).
3. Add a DNS record that points your domain name (devshop.thinkdrop.net) to your server's IP address.
4. Add a second DNS record that points a wildcard subdomain of your domain (*.devshop.thinkdrop.net) to your server's IP 
   address. This allows you to setup new sites without having to mess with DNS every time.
5. Login to your server as root, and retrieve and run the install script:
  ```
  root@devshop:~# wget http://getdevshop.com/install
  root@devshop:~# bash install
  ```
  
*NOTE:* http://getdevshop.com/install simply redirects to the dev version of install.sh: http://drupalcode.org/project/devshop.git/blob_plain/refs/heads/6.x-1.x:...


Usage
-----

Using devshop is a lot like using aegir.

Visit http://devshop.thinkdrop.net in the browser to view the front-end.

SSH into your server as the `aegir` user to access the back-end.

Use drush to access any of your sites.  Use `drush sa` to see the list of available aliases.

Vagrant
-------

There is now a vagrantfile for DevShop that makes for an easy way to test it out and to contribute to the development of DevShop.

It is included in this package. To use, clone this repo and vagrant up:

See README.vagrant.md for more information.

Testing
-------

Very rudimentary testing is happening on TravisCI at http://travisci.org/drupalprojects/devshop

The install script has been tested on:

  - ubuntu 12.04
  - centos 7.0
