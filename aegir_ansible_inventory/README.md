# Ansible Dynamic Inventory for Aegir Servers

This toolset provides an Aegir Hostmaster site with an
"ansible dynamic inventory" compatible endpoint.

The aegir_ansible_inventory.module provides a URL route at /inventory that
outputs an Ansible inventory in JSON form.

The `ansible-inventory.php` script is for use in the `ansible` or `ansible-playbook` commands.


## Setup & Usage

1. Install this module in an Aegir Hostmaster or DevShop site.
2. Copy the "ansible-inventory.php" file to the folder you will call "ansible" from and make sure it is executable.
3. Use ansible to talk to your aegir servers:

    ansible all -i ansible-inventory.php -m command -a 'ls -la'


