# Ansible Role: DevShop Users

This role serves to prepare a server's system users. 

It was originally created to install the `aegir` user for OpenDevShop Servers.

It is being repurposed as a generic "application user" role, as well as being
able to setup system admin users. 

1. Installs an "application" user with special permissions.
  - Initial implementation is the `aegir` user, who has access to run `sudo apachectl`. 
2. *Coming soon:* Recommends `geerlingguy.github-users` and `geerlingguy.security` roles.

Dependencies
------------

None. This is a base role that only deals with linux users.

License
-------

GPL-2

Author Information
------------------

Jon Pugh <jon@thinkdrop.net>