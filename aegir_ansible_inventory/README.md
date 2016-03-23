# Ansible Dynamic Inventory for Aegir Servers

This toolset provides an Aegir Hostmaster site with an
"ansible dynamic inventory" compatible endpoint.

The aegir_ansible_inventory.module provides a URL route at /inventory that
outputs an Ansible inventory in JSON form.

The `ansible-inventory.php` script is for use in the `ansible` or `ansible-playbook` commands.


## Setup & Usage

1. Install this module in an Aegir Hostmaster or DevShop site.
2. Copy the "ansible-inventory.php" file to the folder you will call "ansible" from and make sure it is executable.
<<<<<<< HEAD
2. Set the `AEGIR_HOSTMASTER_HOSTNAME` environment variable to your hostmaster server:

    $ export AEGIR_HOSTMASTER_HOSTNAME=aegir.myhostname.com

=======
3. Ensure that your acting user can SSH into the servers as the "aegir" user. 
>>>>>>> 38a3304c023aa069a47fa98d203c7468b30f0f2d
3. Use ansible to talk to your aegir servers:

    $ ansible all -i ansible-inventory.php -m command -a 'whoami'
    $ ansible db -i ansible-inventory.php -m command -a 'pwd'
    $ ansible http -i ansible-inventory.php -m command -a 'drush status'

See the "Patterns" section in the ansible documentation on more wansiays to target servers: http://docs.ansible.com/ansible/intro_patterns.html

1. Use "all" to run on all servers
2. Use "db" to run on all "db" servers.
3. Use "mysql" to run on all servers using the "mysql" service.

For now, the inventory is locked in to use the 'aegir' user. Once we have a field
on servers we will be able to change this.

## Using Ansible Playbook

We've added a playbook.yml file to this repo in an attempt to allow core aegir servers and services to be configured automatically.

Use the `ansible-playbook` command:

    $ ansible-playbook playbook.yml -i ansible-inventory.php  -u root


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
