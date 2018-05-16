# System Configuration with Ansible

## System Configuration

DevShop is now installed using Ansible roles. This means that most system-level configuration is now managed with Ansible variables.

### Install.sh & Ansible

When you run `install.sh`, it collects information about your system and passes that to the `ansible-playbook` command. The script is idempotent, meaning you can run it many times and get the same result.

These variables are things like `system_hostname`, `mysql_root_password`, or `php_memory_limit`.

When `install.sh` runs, it creates a simple Ansible inventory file in the same folder that `install.sh` resides. This file contains a single entry using just the hostname by default.

If the file already exists, the `install.sh` script will tell ansible-playbook to use that file.

This `inventory` file is the key to customizing the configuration of your server.

#### Default Ansible inventory vs DevShop inventory

DevShop's `install.sh` script uses a separate `inventory` file than the default Ansible inventory \(at `/etc/ansible/hosts`\). This is to remain unobtrusive to the system. In the future, we will likely start using the default inventory system.

Create a file called `inventory` in the same directory as `install.sh`. You can place custom variables in there in the form of an "Ansible Inventory".

### Ansible Inventory Format

For your devshop server's inventory file you do not need to worry about groups. By default, it just includes the hostname.

```text
devshop.mydomain.com
```

If you want to add variables you can do so with this format:

```text
devshop.mydomain.com server_hostname=devshop.mydomain.com php_memory_limit=256M
```

**Explanation of the hostname vs** `server_hostname`**:**

The first hostname mentioned in the file above, "devshop.mydomain.com", is what ansible will use to find the server, using a DNS lookup.

The second, `server_hostname=devshop.mydomain.com` is the variable that Ansible will use to try and set the _system hostname_ of this machine.

### Ansible Variables & Templates

This list of available Ansible variables depends on the roles being used. DevShop uses Jeff Geerling's roles which are very well written, so there are many variables to use.

The easiest way to review all of the roles, variables, and templates that DevShop uses in one place is to use the `ansible-galaxy` command and DevShop's `roles.yml` file: [https://github.com/opendevshop/devshop/blob/1.x/roles.yml](https://github.com/opendevshop/devshop/blob/1.x/roles.yml)

Get that file and run `ansible-galaxy install`:

```text
   ansible-galaxy install -r roles.yml
```

In each Ansible Role repository, look for the `defaults/main.yml` file. In this file are all the available variable names you can use, along with their default values.

Also look for the `templates` folder. Most configuration files on the system come from these templates.

For example, `php_memory_limit` is in the `defaults/main.yml` file located at [https://github.com/geerlingguy/ansible-role-php/blob/master/defaults/main.yml\#L48](https://github.com/geerlingguy/ansible-role-php/blob/master/defaults/main.yml#L48) and is written to the `templates/php.ini.j2` file at [https://github.com/geerlingguy/ansible-role-php/blob/master/templates/php.ini.j2\#L36](https://github.com/geerlingguy/ansible-role-php/blob/master/templates/php.ini.j2#L36)

### Configuring your System

You can edit your `inventory` file, then run `install.sh` again to reconfigure your system. The Ansible playbooks are smart enough to restart services if a configuration file changes.

## More Ansible with Aegir

We have developed a module that allows Aegir to become your Ansible Inventory. It is possible to setup your DevMaster front-end to manage the server's inventory and variables.

Enable Aegir Ansible Inventory & Variables module, and read up on the documentation. [http://cgit.drupalcode.org/aegir\_ansible/tree/README.md?h=7.x-1.x](http://cgit.drupalcode.org/aegir_ansible/tree/README.md?h=7.x-1.x)

