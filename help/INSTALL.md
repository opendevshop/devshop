Installing DevShop
==================

DevShop is installed with a standalone <a href="https://raw.githubusercontent.com/opendevshop/devshop/0.x/install.sh">install.sh</a> script.

The script is designed to work on any Debian or RedHat/CentOS server. Please file issues if you have any problems with it.

Setup
-----

- Pick a domain and server name to use for DevShop, for example "devshop.thinkdrop.net"
- Fire up a linux server somewhere, using that domain name as the server's hostname. (Ubuntu 12 or 14 are most likely to work without issue, however we want to support most Linux OS. If you have problems in other OS please submit an issue.)
  - Rackspace and DigitalOcean use the name of the server to automatically set the hostname.
  - On Amazon Web Services <a href="http://docs.aws.amazon.com/AWSEC2/latest/UserGuide/set-hostname.html">you must change the hostname manually</a>.
  - On Linode, you must also <a href="https://www.linode.com/docs/getting-started#setting-the-hostname">set the hostname manually</a>.

- Add a DNS record that points your domain name (devshop.thinkdrop.net) to your server's IP address.
- Add a second DNS record that points a wildcard subdomain of your domain (*.devshop.thinkdrop.net) to your server's IP address. This allows you to setup new sites without having to mess with DNS every time.
- Login to your server as root, and retrieve and run the install script:

  ```
  root@devshop:~# wget https://raw.githubusercontent.com/opendevshop/devshop/0.2.2/install.sh
  root@devshop:~# bash install
  ```
Chasing Head
------------

The 0.x branch install script will install the latest devshop from git source.

Use https://raw.githubusercontent.com/opendevshop/devshop/0.x/install.sh if you wish to do this.

Many people install this version.  Updates are done with a simple `git pull` in ~/devmaster-0.x/profiles/devmaster

Install Script
--------------

The install script (install.sh) is only needed to prepare the server to provision itself with Ansible.

It is designed to run as a standalone script.

### Install Script Overview

1. Installs git and Ansible.
2. Generates a secure MySQL password and saves it to the /root/.my.cnf.
3. Clones http://github.com/opendevshop/devshop.git to /usr/share/devshop.  These files include the Ansible playbooks and variables files.
4. Runs the Ansible playbook.
5. Outputs a link to login to the devshop front-end.

### Ansible Playbook Overview

The Ansible playbook is located in the devshop repo at ./playbook.yml.

Ansible is human readable, so if you are interested in what happens there, just open that file and read it.


