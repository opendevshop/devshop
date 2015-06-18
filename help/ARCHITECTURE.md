DevShop Architecture
====================

Users
-----

DevShop is built on Aegir, so it uses the `aegir` system user for all activity.

It is not recommended to use the `root` user after the initial server setup.  Everything short of installing new server packages can be done with the `aegir` user.

The `aegir` user's only permission is to reload apache, nginx, and tomcat.  It is granted this permission via `/etc/sudoers.d/aegir` so that it can run `sudo /etc/apache2ctl` without being asked for a password.

The `aegir` user does not have a password set by default. This is recommended. If access needs to be granted, use SSH keys to allow people to login as the `aegir` user. 

The `aegir` user's home directory is `/var/aegir`.  All apache config files and site source code files, including devmaster, are located in this folder.

During typical drupal site development, developers need access to the server via SSH.  This may be granted via the `aegir` user by placing developers SSH public keys into `/var/aegir/.ssh/authorized_keys`.

Be aware that _all_ sites on the server are stored under the single `aegir` user. Granting access to someone gives them access to all of the sites on that server.  It is possible to use the "clients" feature of aegir to create sub-users and grant them access to only their folder in `/var/aegir/clients/` however this is difficult and outside of the scope of devshop.

Apache
------

Aegir uses Apache web server by default.  NGINX is also possible.

Aegir works because on setup, a symlink is placed into the Apache config folder that points to `/var/aegir/config/apache`, which in turn includes all files in `/var/aegir/config/server_master/apache`.  

This way, `aegir` user can write apache configs without needing root access.

Once setup, aegir writes it's own configs for additional sites into it's own `/var/aegir/config/server_master/config` folder, then reloads apache.

MySQL
-----

Aegir stores the root password for the MySQL servers it has access to.  This is so it can create and destroy new databases for the sites it is creating.

For each new site, a new database, user and password is created so that sites do not have to share credentials.

The access credentials are stored in each site's apache VirtualHost configuration so they are not available in the site's source code.

SSH
---

The `aegir` user is what connects to your git repos as well as to your remote servers.

DevShop creates a unique SSH keypair for each devshop install.  You must use that keypair to grant access to your repositories, and to remote servers.

Ports
-----

Aegir & devshop expose default ports for HTTP and SSH (80 and 22).  It makes no assumptions about what network security you would like beyond that. Installing extra firewalls is up to you.

Config Manager
--------------

Aegir is essentially an apache config manager.  

When a new site is created, the following occurs: 

1. A new apache VirtualHost configuration file is added to `/var/aegir/config/server_master/apache/vhost`.
2. A new database is created.
3. Database credentials are written to the site's VirtualHost config file to make them available as `$_SERVER` variables in `settings.php`.  
4. Within the drupal codebase, the `sites/DOMAIN.com` folder is created, along with a `settings.php` file and the `files` folder.  The permissions for all of these files are appropriately set automatically.  The `settings.php` is created from a template that is stored in the provision drush project.  A special hook can be used to add lines to the settings.php file.
5. Apache is reloaded.
6. The site is installed via drush, using the selected install profile.

Remote Servers
--------------

Aegir has the ability to connect and deploy to remote servers.

Each server gets a folder created in `/var/aegir/config/server_NAME`.  Inside this folder is the same collection of folders present in `server_master`.

Remote servers must have Apache installed, and must have their own Aegir user with the same extra sudo permissions described above, as well as the same symlink in apache config.  You must also install MySQL and setup the root user to be accessible from the master server.

The master server must have SSH access to the remote servers, from `aegir` user to `aegir` user.

Once this is setup, visit the "Add Server" page in the devshop/aegir front-end. Enter a hostname that resolves to the server's IP address.  You must choose the services, apache & mysql 

During the "server verify" process, all of the files in `/var/aegir/config/server_NAME` are copied to the remote server at the same path via RSYNC.

When new sites are created, and the new server is selected as the target, the site codebase will be copied to the remote server at the same path via RSYNC.  This happens again during the "site verify" process.