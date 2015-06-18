DevShop Architecture
====================

Users
-----

DevShop is built on Aegir, so it uses the `aegir` system user for all activity.

The `aegir` user's only permission is to reload apache, nginx, and tomcat.  It is granted this permission via `/etc/sudoers.d/aegir` so that it can run `sudo /etc/apache2ctl` without being asked for a password.

The `aegir` user's home directory is `/var/www`.  All apache config files and site source code files, including devmaster, are located in this folder.

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

Aegir & devshop expose default ports for HTTP and SSH (80 and 22) respectively.  It makes no assumptions about what network security you would like beyond that. Installing extra firewalls is up to you.

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