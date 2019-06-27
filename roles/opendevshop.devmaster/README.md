# Ansible Role: DevShop Devmaster

[![Build Status](https://travis-ci.org/opendevshop/ansible-role-devmaster.svg?branch=master)](https://travis-ci.org/opendevshop/ansible-role-devmaster)

Prepares a server for hosting Drupal with the DevShop system.

Devmaster is the Drupal install profile that serves as the web and REST interface for DevShop.

The Drupal code for Devmaster is located at [drupal.org/project/devmaster](drupal.org/project/devmaster).

Requirements / Dependencies
------------

This role depends on the following addiitonal roles:

- [opendevshop.aegir-user](https://galaxy.ansible.com/opendevshop/aegir-user)
- [opendevshop.aegir-apache](https://galaxy.ansible.com/opendevshop/aegir-apache)
- [geerlingguy.composer](https://galaxy.ansible.com/geerlingguy/composer)
- [geerlingguy.php](https://galaxy.ansible.com/geerlingguy/php)
- [geerlingguy.php-mysql](https://galaxy.ansible.com/geerlingguy/php-mysql)
- [geerlingguy.mysql](https://galaxy.ansible.com/geerlingguy/mysql)

See the [DevShop Playbook.yml](https://github.com/opendevshop/devshop/blob/1.x/playbook.yml) file for an example playbook.

Role Variables
--------------

Available variables are listed below, along with default values (see `defaults/main.yml`):

    server_hostname: local.devshop.site

The hostname to set for this server. The hostname should match a fully-qualified domain name that will resolve to the server you are using.

If using the install.sh script, this variable is either set from the `--hostname` option, or automatically detected from the `hostname -f` command.

*NOTE:* You could use Ansible to create the DNS records with your own playbook. See [DNSimple](https://docs.ansible.com/ansible/latest/modules/dnsimple_module.html), [DigitalOcean Domains](https://docs.ansible.com/ansible/latest/modules/digital_ocean_domain_module.html), or [Route53](https://docs.ansible.com/ansible/latest/modules/route53_module.html), [Azure DNS](https://docs.ansible.com/ansible/latest/modules/azure_rm_dnsrecordset_module.html#azure-rm-dnsrecordset-module), or [other](https://docs.ansible.com/ansible/latest/modules/list_of_all_modules.html?highlight=DNS) Ansible modules.

    devshop_devmaster_email: admin@devshop.local.computer
    
The email address to use for the Devmaster Dashboard user account #1. Default to `admin@devshop.local.computer`

    devshop_cli_repo: http://github.com/opendevshop/devshop.git
    
The git repository to use for the CLI and Ansible roles data. 

    devshop_cli_path: /usr/share/devshop
    
The path to install the CLI code to. 

    devshop_cli_update: yes
    
Set to "no" to block updating the `devshop_cli_path` to the `devshop_version`.


### install.sh script

The recommended way to install DevShop is with the [install.sh](https://github.com/opendevshop/devshop/blob/1.x/install.sh)
 
 script, but these roles also work if the variables are set correctly.

The install script prepares certain variables and runs this playbook.


Example Playbook
----------------

The devshop install.sh script uses this [playbook.yml](https://github.com/opendevshop/devshop/blob/1.x/playbook.yml) file:

```yml
##
# DevShop: DevMaster Server with Apache
#

---
- hosts: all
  user: root
  roles:
    - opendevshop.aegir-user
    - opendevshop.aegir-apache
    - geerlingguy.php
    - geerlingguy.php-mysql
    - geerlingguy.composer
    - opendevshop.devmaster
```

License
-------

GPL-2

Author Information
------------------

Jon Pugh <jon@thinkdrop.net>