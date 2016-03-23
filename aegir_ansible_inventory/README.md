# Ansible Dynamic Inventory for Aegir Servers

This toolset provides an Aegir Hostmaster site with an
"ansible dynamic inventory" compatible endpoint.

The aegir_ansible_inventory.module provides a URL route at /inventory that
outputs an Ansible inventory in JSON form.

The `ansible-inventory.php` script is for use in the `ansible` or `ansible-playbook` commands.


## Setup & Usage

1. Install this module in an Aegir Hostmaster or DevShop site.
2. Copy the "ansible-inventory.php" file to the folder you will call "ansible" from and make sure it is executable.
2. Set the `AEGIR_HOSTMASTER_HOSTNAME` environment variable to your hostmaster server:

    $ export AEGIR_HOSTMASTER_HOSTNAME=aegir.myhostname.com

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
