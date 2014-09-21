Ansible DevShop Installer
=========================

This playbook works in both the Debian and RedHat Families of Linux.

To run this playbook, you must have ansible available either on your local
machine or on the server you are provisioning.

Installing Ansible on-server
----------------------------

Ansible is just a command so there is no overhead to having it running on your devshop server.

To setup a devshop server from scratch, you must have a domain name that resolves to your debian or redhat server. The ansible playbook will set the hostname for you.

ssh as root in and follow these instructions:

1. Install Ansible & Git.

  In Debian systems, the easiest way to install ansible currently is with `pip`.

  ```
  # apt-get install git python-apt python-pycurl
  # pip install ansible
  ```

  In RedHat Systems, you can just use yum if you also install epel-release

  ```
  # yum install git epel-release
  # yum install ansible
  ```

2. Clone this repo, & create your `inventory` file.

  ```
  $ git clone http://github.com/drupaldevshop/install-ansible.git
  $ echo $HOSTNAME > inventory
  ```
  Edit the vars.yml file to set server_hostname and any other vars you wish to change, and run `ansible-playbook` with a local connection:

  ```
  $ vi vars.yml
  ```

3. Run `ansible-playbook`:

  ```
  $ sudo ansible-playbook -i inventory playbook.yml --connection=local --sudo
  ```

  At the end, if it does not fail, you will see a one time login URL.

vars.yml
--------

When deploying to a real server, you must change some values in vars.yml.

- Change `server_hostname` to match the domain name you will use to access the
 devshop front-end.
- Change `mysql_root_password` to a long random string to secure it.
- Change `php_timezone` to match your timezone.  See http://php.net/manual/en/timezones.php
- You can change `devshop_version` to decide what version of devshop to install.  Recommend you use 6.x-1.x until the first beta.

Vagrant
-------

There is a Vagrantfile in this repo with two VM's defined: `debian` and `redhat`.

You can fire up both with `vagrant up`.

You can fire up just one with `vagrant up debian`.

NOTE: We are using the basebox `chef/centos-7.0` for the `redhat` VM.  This box seems to error out on `vagrant up`. Simply run `vagrant provision` to run the installer once that happens.

