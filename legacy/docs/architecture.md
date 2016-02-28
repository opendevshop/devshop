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

Be aware that _all_ sites on the server are stored under the single `aegir` user. Granting access to someone gives them access to all of the sites on that server.  It is possible to use the "clients" feature of aegir to create sub-users and grant them access to only their folder in `/var/aegir/clients/` however this is difficult and outside of the scope of devshop, for now.

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

Upon install, DevShop creates a  `/var/aegir/.ssh/config` file that sets `StrictHostKeyChecking` to `no` for common git hosts.  This is so site codebases can be cloned automatically without manual approval of the host being needed.

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

Deploy Methods
--------------

There are three "Deploy Methods" that are possible for each project:

1. Immediate Deployment:

  Runs deployment the moment that a webhook request is received.  The devshop front-end must be exposed to the internet for this method to work.  
  
  To use Immediate Deployment, you must configure a "webhook" with your git provider. This notifies your devshop server whenever code is pushed to the git provider.
  
2. Queued Deployment:

  Runs deployment every 1 minute (configurable).  This method would usually be used only when devshop cannot be exposed to the internet.  
  
3. Manual Deployment:

  Code is only deployed manually, through the devshop front-end.  

Git Integration
---------------

The mechanism by which DevShop gets code onto the servers is called the "Deploy" task.

A "Deploy" task will checkout the chosen branch or tag, and runs `git pull` if on a branch.

After the `git pull`, "Deploy Hooks" are run immediately.  What deploy hooks run is configured in the Environment Settings in the devshop front-end. 

Deploy Hooks
------------

The Deploy Hooks available out of the box are:

1. Update Database
2. Clear Caches
3. Revert All Features

Deploy hooks can be added via drush include files.

Also supported are Acquia "Cloud Hooks". These are files contained in the repo that are used by Acquia Cloud hosting.

Environments, Git, & Release Strategies
---------------------------------------

Each environment can be configured to track a branch or a tag.

If set to "Immediate Deployment" and the git webhook is set up, environments that are set to track a branch will be updated whenever a `git push` to that branch occurs.

If an environment is set to a tag, the environment will stay at that tag until changed.

With this configuration, you can employ a number of different release strategies.

1. Tagged Release for Production & Stage
 
  The most controlled release process will involve using Tags for releasing to your Staging environment and your Production environment.
  
  You should create release tags such as `v1.0.0` and increment them as your code becomes ready.  
  
2. Branch Releases for Production
  
  If you set your prodution environment to a branch such as `live`, any push to that branch will be deployed automatically. If you use this setup you should have a good testing process in place.