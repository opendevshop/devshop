DevShop
=======

Welcome to the DevShop Source code.

Contents
--------

This project contains the four things needed to build a DevShop:

1. install.sh
  A bash script to go from zero to DevShop.  All you need is a new Ubuntu
  12.04 server and this script.

2. devshop.make
  The makefile used to build a DevShop instance.  It does NOT include itself as
  a profile

3. build-devshop.make
  The makefile used by install-devshop.sh to build a DevShop instance. It is called from the drupal.org git server before anything is cloned.

3. devshop.profile
  The Drupal installation profile for DevShop.

Installation
------------

1. Get a new Ubuntu 12.04 LTS (or reasonable equivalent).
2. Set the hostname to match the fully qualified domain you would like to use
 for your DevShop front-end.
3. Add a DNS A record to point that domain to the IP address of your server.
4. Run the DevShop Install script. (install.sh)

    wget http://drupalcode.org/project/devshop.git/blob_plain/refs/heads/6.x-1.x:/install.sh
    sudo sh install.sh

The only question you will have to answer is the Postfix configuration.  The
default options are fine:

  1. Internet Host
  2. yourhostname.com

@TODO: Preseed the postfix config

Usage
-----

Using devshop is a lot like using aegir.

Visit http://yourhostname in the browser to view the front-end.

SSH into your server as the `aegir` user to access the back-end.

Use drush to access any of your sites.  Use `drush sa` to see the list of available aliases.

Testing
-------

Very rudimentary testing is happening on TravisCI at http://travisci.org/drupalproject/devshop

