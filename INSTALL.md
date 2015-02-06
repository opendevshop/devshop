Installing DevShop
==================


The fastest way to install DevShop is with the new standalone <a href="http://drupalcode.org/project/devshop.git/blob_plain/refs/heads/6.x-1.x:/install.sh">install.sh</a> script.

The script is designed to work on any Debian or RedHat/CentOS server. Please file issues if you have any problems with it.

Setup
-----

- Pick a domain and server name to use for DevShop, for example "devshop.thinkdrop.net"
- Fire up a linux server somewhere, using that domain name as the server's hostname. (Ubuntu 12 or 14 are most likely to work without issue, however we want to support most Linux OS. If you have problems in other OS please submit an issue.)
  - Rackspace and DigitalOcean use the name of the server to automatically set the hostname.
  - On Amazon Web Services <a href="http://docs.aws.amazon.com/AWSEC2/latest/UserGuide/set-hostname.html">you must change the hostname manually</a>.
  - On Linode, you must also <a href="https://www.linode.com/docs/getting-started#setting-the-hostname">set the hostname manually</a>.

- Add a DNS record that points your domain name (devshop.thinkdrop.net) to your server's IP address.
- Add a second DNS record that points a wildcard subdomain of your domain (*.devshop.thinkdrop.net) to your server's IP address. This allows you to setup new sites without having to mess with DNS every time.
- Login to your server as root, and retrieve and run the install script:

  ```
  root@devshop:~# wget http://getdevshop.com/install
  root@devshop:~# bash install
  ```

  *NOTE: http://getdevshop.com/install simply redirects to the dev version of install.sh: http://drupalcode.org/project/devshop.git/blob_plain/refs/heads/6.x-1.x:/install.sh*