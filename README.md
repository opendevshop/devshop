# Ansible Dynamic Inventory for Aegir Servers

This toolset allows for tight integration between Ansible & Aegir.

It involves a few components:

- **Aegir Ansible Inventory module** is a simple Drupal module that provides a URL at http://hostmaster/inventory with JSON data in an Ansible Dynamic Inventory format.
- **Aegir Ansible Services** provide Aegir compatible MySQL and Apache Services, configured with Ansible.
- **Aegir Ansible Roles** allows aegir users to select from a configurable list of roles for each server.
- **Aegir Ansible Variables** allows aegir users to configure custom Ansible variables per server.

  Our roles simply add on to existing Ansible Galaxy Roles to provide standardized Apache, MySQL, NGINX, and PHP configuration.

  We have 3 roles so far:

  - `aegir.user` role prepares the aegir user and SSH access.
  - `aegir.apache` role applies the needed configuration to apache, and sets the sudo permissions needed to restart it.
  - `aegir.nginx` role does the same for nginx.

- **Ansible `inventory` script** is used to replace a static ansible inventory file.

  Both `ansible` and `ansible-playbook` commands have the `-i` or `--inventory-file` option. This is the path to your system's "inventory file".  An ansible inventory file is similar to a `/etc/hosts` file: it lists servers, but it is much more powerful.

  If your inventory file is executable, `ansible` and `ansible-playbook` commands will run the inventory as a script.  This script must return JSON data compatible with Ansible's *Dynamic Inventory* feature.

  What our `inventory` script does is reach out to http://hostmaster/inventory and return the results.

  This file should replace your default ansible hosts file at `/etc/ansible/hosts` and be made executable.

  Further Reading on Ansible Dynamic Inventory:

  - http://docs.ansible.com/ansible/intro_dynamic_inventory.html
  - http://docs.ansible.com/ansible/developing_inventory.html
  - http://www.jeffgeerling.com/blog/creating-custom-dynamic-inventories-ansible


## Setup & Usage
1. Install this module in an Aegir Hostmaster or DevShop site. Install it in the `sites/HOSTNAME/modules` path so the module remains on upgrade.  (as `aegir`, for easy install.)

        aegir@hostmaster:~$ git clone http://github.com/opendevshop/aegir_ansible /var/aegir/hostmaster-7.x-3.x/sites/HOSTNAME/modules
        aegir@hostmaster:~$ drush @hostmaster en aegir_ansible_inventory

   *Everything else can be done on any server that can access the http://hostmaster/inventory URL, and has SSH access to the servers you wish to configure.*

   You can even install ansible, the inventory and playbook files on the new remote server itself.  Instead of installing over SSH access, you can run `ansible-playbook` as root, and use the `--connection local` option to simply run the playbooks in place.

2. Copy the `inventory` file to `/etc/ansible/hosts` and make it executable (as `root`, or use sudo. Note `aegir` user cannot sudo.):

        root@local:~# cp /var/aegir/hostmaster-7.x-3.x/sites/HOSTNAME/modules/aegir_ansible/inventory /etc/ansible/hosts
        root@local:~# chmod +x /etc/ansible/hosts

   When using our dynamic inventory file, it assumes your hostname is available as a FQDN.  If your server's hostname is `aegir.mysite.com`, then it will load `http://aegir.mysite.com/inventory` by default.

   If your hostname does not match your available FQDN, you can set the `AEGIR_HOSTMASTER_HOSTNAME` environment variable or you can just directly edit `/etc/ansible/hosts` if you can remember to edit it again if there are upgrades.

    To set an environment variable, either type this out or put it in `/etc/bashrc` or `/etc/bash.bashrc`

        export AEGIR_HOSTMASTER_HOSTNAME=aegir.myhostname.com
ansible all -u root --become --become-user=aegir -m command -a 'drush @hm cc all'

5. Install Galaxy Roles

  There is a `roles.yml` file that includes the needed galaxy roles.  Use the `ansible-galaxy` command to install them:

        root@local:~# cd /var/aegir/hostmaster-7.x-3.x/sites/HOSTNAME/modules/aegir_ansible
        root@local:~# ansible-galaxy install -r roles.yml

   If this fails you can install the roles "manually" (as root, so they are installed globally)

        ansible-galaxy install geerlingguy.apache
        ansible-galaxy install geerlingguy.composer
        ansible-galaxy install geerlingguy.drush
        ansible-galaxy install geerlingguy.git
        ansible-galaxy install geerlingguy.mysql
        ansible-galaxy install geerlingguy.nginx
        ansible-galaxy install geerlingguy.php
        ansible-galaxy install geerlingguy.php-mysql

   And install our "custom" roles.  (Once we publish the roles to ansible galaxy, this won't be needed).

        git clone http://github.com/opendevshop/ansible-role-aegir-user /etc/ansible/roles/aegir.user
        git clone http://github.com/opendevshop/ansible-role-aegir-apache /etc/ansible/roles/aegir.apache
        git clone http://github.com/opendevshop/ansible-role-aegir-nginx /etc/ansible/roles/aegir.nginx

4. Ensure SSH access.
Ensure that your acting user can SSH into the servers.  You can either access as root or as another user that can sudo.
Type `ansible` and look for the `-u and --become options for more info about how ansible connects.

5. Use ansible to talk to your aegir servers.  You can refer to a single server or a group of servers using the following "patterns":

  - Service Type: http, db
  - Service: apache, nginx, mysql
  - Hostname: localhost, aegir.myserver.com
  - "Hosting Context": server_master

  Some example commands:

        $ ansible all -m command -a 'whoami'
        $ ansible db -m command -a 'pwd'
        $ ansible mysql -m command -a 'pwd' -u root
        $ ansible http -m command -a 'drush '
        $ ansible apache -m command -a 'service apache2 restart' -u root
        $ ansible aegir.serverhostname.com -m command -a 'drush @hostmaster uli' -u aegir

  The first and only argument is required for ansible, it's called a "pattern". You can specify "all" to run on all
  servers, or you can specify a service type (db, http) or a service name (mysql, apache, nginx)
  or a hostname (localhost, aegir.servermaster.com).

  See the "Patterns" section in the ansible documentation on more ways to target servers: http://docs.ansible.com/ansible/intro_patterns.html

  The "-u" option determines what user ansible tries to login as.  If you need to perform operations that require
  root privileges, specify "-u root".  Otherwise, ansible will default the name you are currently using (the same
  behavior for the 'ssh' command.)

  If you have root access, but want to run a command as a different user (such as aegir) you can use --become and --become-user options:
    
          $ ansible all -u root --become --become-user=aegir -m command -a 'drush @hm cc all'

## Using Ansible Playbook

We've added a playbook.yml file to this repo in an attempt to allow core aegir servers and services to be configured automatically.

Use the `ansible-playbook` command:

    $ ansible-playbook playbook.yml -u root

You can run the plays on a subset of servers in your inventory by using the "-l" option and a _pattern_.

    $ ansible-playbook playbook.yml -u root --list-hosts -l aegir.remote

### Roles

Install the required galaxy roles using the `install-roles.yml` file:

    $ ansible-galaxy install -r install_roles.yml

## Aegir User Role & SSH Key

Aegir remote servers need to have SSH access and an aegir user, so we've created a role for "aegir-user".

This role includes setting up the `authorized_keys` file for the remote aegir user.

The `aegir_ansible_inventory.module` includes the hostmaster's public key in the server variables automatically, as long as the variable `devshop_public_key` is set. (This happens automatically in devshop. You will have to set it manually in Aegir).

 You may also pass the public key in via the command line instead of relying on the inventory, using the `--extra-vars` option.

    $ ansible-playbook playbook.yml -l devshop.remote -u root --extra-vars "aegir_user_authorized_keys='ssh-rsa AAAAaaaa aegir@server_master'"

The `aegir_user_authorized_keys` should be set as the aegir@server_master user's public SSH key.

# Next Steps

Now that we have have an inventory and groups and a playbook associating the groups with ansible roles, we have the
framework we need to add the final layer: aegir.apache, aegir.nginx, and aegir.mysql roles that setup the last bits
of configuration to make the servers usable as aegir remotes.

This is the first step toward having "node/add/server" in Aegir actually fully bootstrap the servers.

A requirement of that is tracking what "user" the server is allowing us to connect as.  In our opinion this should be added to aegir core.

We will need to connect as root to provision servers.   

The recommended workflow would be to have a separate user on your server that has SSH access as root or as a user that can "sudo". We are developing these best practices now and will use this repo to codify it.

Stay tuned!
