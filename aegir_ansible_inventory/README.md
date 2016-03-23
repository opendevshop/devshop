# Ansible Dynamic Inventory for Aegir Servers

This toolset provides an Aegir Hostmaster site with an
"ansible dynamic inventory" compatible endpoint.

The aegir_ansible_inventory.module provides a URL route at /inventory that
outputs an Ansible inventory in JSON form.

The `ansible-inventory.php` script is for use in the `ansible` or `ansible-playbook` commands.


## Setup & Usage

1. Install this module in an Aegir Hostmaster or DevShop site.
2. Copy the "ansible-inventory.php" file to the folder you will call "ansible" from and make sure it is executable.
3. Ensure that your acting user can SSH into the servers as the "aegir" user. 
3. Use ansible to talk to your aegir servers:

    $ ansible all -i ansible-inventory.php -m command -a 'whoami'
    $ ansible db -i ansible-inventory.php -m command -a 'pwd'
    $ ansible http -i ansible-inventory.php -m command -a 'drush status'

1. Use "all" to run on all servers
2. Use "db" to run on all "db" servers.
3. Use "mysql" to run on all servers using the "mysql" service.

For now, the inventory is locked in to use the 'aegir' user. Once we have a field
on servers we will be able to change this.

## Next Steps

Now that we have have an inventory and groups, we should be able to associate ansible roles with aegir services.

This is the first step toward having "node/add/server" in Aegir actually fully bootstrap the servers.

A requirement of that is tracking what "user" the server is allowing us to connect as.  In our opinion this should be added to aegir core.

We will need to connect as root to provision servers.   

The recommended workflow would be to have a separate user on your server that has SSH access as root or as a user that can "sudo". We are developing these best practices now and will use this repo to codify it.

Stay tuned!
