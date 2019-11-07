# Ansible Role: DevShop Apache

Extends the geerlingguy.apache role for DevShop.

- Allows `app-user` to run `apachectl` to allow dynamic reconfiguration
of Apache by the application by adding sudoers.
- Symlink from /etc/apache/conf.d to include Apache config in the `app-user`
 home directory.
 
Dependencies
------------

- [geerlingguy.apache](https://galaxy.ansible.com/geerlingguy/apache)

License
-------

GPL-2

Author Information
------------------

Jon Pugh <jon@thinkdrop.net>