
DevShop
=======

This project contains the three things needed to build a DevShop:

1. install-devshop.sh
  A bash script to go from zero to DevShop.  All you need is a new Ubuntu
  12.04 server and this script.
2. devshop.make
  The makefile used to build a DevShop instance.  It does NOT include itself as
  a profile
3. devshop.profile
  The Drupal installation profile for DevShop.

Installation
------------

1. Get a new Ubuntu 12.04 LTS (or reasonable equivalent).
2. Set the hostname to match the fully qualified domain you would like to use
 for your DevShop front-end.
3. Add a DNS A record to point that domain to the IP address of your server.
4. Run the DevShop Install script. (install-devshop.sh)

    wget http://drupalcode.org/project/devshop.git/blob_plain/refs/heads/6.x-1.x:/install.debian.sh
    sudo sh install.debian.sh

The only question you will have to answer is the Postfix configuration.  The
default options are fine:

  1. Internet Host
  2. yourhostname.com

@TODO: Preseed the postfix config
