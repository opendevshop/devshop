# Changing Hostname

Sometimes you need to change your devshop's hostname.

Here's the process.

1. Rename your server in your cloud provider.  Not technically required, but it will reduce confusion.
2. Add a DNS record for your new name to point to your server's IP.
3. Edit /etc/hostname and enter your new full hostname.
4. Edit /etc/hosts and change the old hostname to the new one.
5. In your devmaster front end, find the site node for your front-end, and add the new domain as an alias. Set the aliases to redirect to your desired URL.
6. At this point you should be able to visit [http://yournewhostname/](http://yournewhostname/) and access the devshop front-end.
7. In your devmaster front-end, all old projects will retain their old base url.  You can edit the database table {hosting\_devshop\_projects} to manually update the base URL so that new environments will be created under the new hostname.
8. Reboot the server.

