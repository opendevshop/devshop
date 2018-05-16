# How it works

## Technical details of DevShop & Aegir

### Summary

DevShop & Aegir leverage the standard LAMP \(Linux-Apache-MySQL-PHP\) stack to automate the preperation and installation of Drupal.

Provision, the back-end CLI for both Aegir & DevShop, does the hard work of writing apache/nginx Virtual Host configuration files, setting Drupal file permissions, creating database users and setting permissions, writing Drush alias files, backing up databases, restarting apache, and more.

The GUI for Aegir and DevShop is yet another Drupal site, provisioned using the same methods on the other hosted sites. Aegir's GUI is called Hostmaster and DevShop is called Devmaster. Both are Drupal Distributions hosted on Drupal.org. See [https://www.drupal.org/project/hostmaster](https://www.drupal.org/project/hostmaster) and [https://www.drupal.org/project/devmaster](https://www.drupal.org/project/devmaster) for more information.

The Hostmaster or Devmaster instance on an Aegir server is no different from the other hosted sites, except it has the Hosting Queue: a list of tasks to be run via the command line, which is invoked via a service daemon called the Hosting Queue Daemon. This daemon ensures that tasks are running as soon after they are triggered as possible.

The tasks must be run using the back-end CLI \(as the `aegir` user\) because they leverage Provision to write Apache virtualhosts and databases, which should never be allowed from the front-end.

### The `aegir` user.

Apache and nginx run as their standard users \(typically www-data, apache, or nginx\).

There is a special application user called `aegir` that has no extra permissions, except for the ability to call `sudo apache2ctl` or `sudo nginx` without a password, and the ability to connect to the root user of a MySQL database server.

This user is also in the web-user group, so that it can change files to be in that group.

### "Server Contexts"

Aegir stores the information it needs to provision Drupal in special files called "Contexts". Each server, platform, and site is stored in a "context". In Provision 3.x, drush alias files are used. In Provision 4, YML files.

By default, Aegir comes with 2 server contexts: one named after the hostname of the server \(for apache\) and one for "localhost" \(for mysql\). The server that provision is running on is always called `server_master`.

### Web Server Configuration

Aegir works by injecting into Apache or NGINX config with a symbolic link.

Typically:

```text
/etc/apache2/conf-enabled/aegir.conf -> /var/aegir/config/apache.conf
```

The file `/var/aegir/config/apache.conf` contains includes to folders for the current server:

```text
# ...
# other configuration, not touched by aegir
# this allows you to override aegir configuration, as it is included before
Include /var/aegir/config/server_master/apache/pre.d
# virtual hosts
Include /var/aegir/config/server_master/apache/vhost.d
# platforms
Include /var/aegir/config/server_master/apache/platform.d
# other configuration, not touched by aegir
# this allows to have default (for example during migrations) that are eventually overriden by aegir
Include /var/aegir/config/server_master/apache/post.d
```

All of the files in these folders is included into Apache or NGINX.

