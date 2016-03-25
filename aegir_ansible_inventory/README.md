# Ansible Dynamic Inventory for Aegir Servers

This toolset provides an Aegir Hostmaster site with an
"ansible dynamic inventory" compatible endpoint.

The aegir_ansible_inventory.module provides a URL route at /inventory that
outputs an Ansible inventory in JSON form.

The `ansible-inventory.php` script is for use in the `ansible` or `ansible-playbook` commands.


## Setup & Usage

1. Install this module in an Aegir Hostmaster or DevShop site.
2. Copy the "ansible-inventory.php" file to the folder you will call "ansible" from and make sure it is executable.
3. Set the `AEGIR_HOSTMASTER_HOSTNAME` environment variable to your hostmaster server:

    $ export AEGIR_HOSTMASTER_HOSTNAME=aegir.myhostname.com

4. Ensure that your acting user can SSH into the servers as the "aegir" user.
5. Use ansible to talk to your aegir servers:

    $ ansible all -i ansible-inventory.php -m command -a 'whoami'
    $ ansible db -i ansible-inventory.php -m command -a 'pwd'
    $ ansible mysql -i ansible-inventory.php -m command -a 'pwd' -u root
    $ ansible http -i ansible-inventory.php -m command -a 'drush '
    $ ansible apache -i ansible-inventory.php -m command -a 'service apache2 restart' -u root
    $ ansible aegir.serverhostname.com -i ansible-inventory.php -m command -a 'drush @hostmaster uli' -u aegir

  The first argument is required for ansible, it's called a "pattern". You can specify "all" to run on all
  servers, or you can specify a service type (db, http) or a service name (mysql, apache, nginx)
  or a hostname (localhost, aegir.servermaster.com).

  See the "Patterns" section in the ansible documentation on more ways to target servers: http://docs.ansible.com/ansible/intro_patterns.html

  The "-i ansible-inventory.php" tells ansible to use our little php script here to return the inventory data.
  The `ansible-inventory.php` file in turn loads the JSON from the http://hostmaster.aegir.server/inventory endpoint.

  The "-u" option determines what user ansible tries to login as.  If you need to perform operations that require
  root privileges, specify "-u root".  Otherwise, ansible will default the name you are currently using (the same
  behavior for the 'ssh' command.)

## Using Ansible Playbook

We've added a playbook.yml file to this repo in an attempt to allow core aegir servers and services to be configured automatically.

Use the `ansible-playbook` command:

    $ ansible-playbook playbook.yml -i ansible-inventory.php  -u root

You can run the plays on a subset of servers in your inventory by using the "-l" option and a _pattern_.

    $ ansible-playbook playbook.yml -i ansible-inventory.php -u root --list-hosts -l aegir.remote

### Roles

Install the required galaxy roles using the `install-roles.yml` file:

    $ ansible-galaxy install -r install_roles.yml

## Aegir User Role & SSH Key

Aegir remote servers need to have SSH access and an aegir user, so we've created a role for "aegir-user".

This role includes setting up the `authorized_keys` file for the remote aegir user, but you must pass it on the command
line in the `--extra-vars` option.

    $ ansible-playbook playbook.yml -l devshop.remote -i ansible-inventory.php  -u root --extra-vars "aegir_user_authorized_keys='ssh-rsa AAAAaaaa aegir@server_master'"

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
