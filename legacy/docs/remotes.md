DevShop Remote Servers
======================

We have created a simple command to setup remote servers:

`devshop remote:install`

You can test this with the Vagrantfiles available in this repo:

1. Clone this repo.
2. Run `vagrant up remote` to turn on the remote server.  You must do this first.
3. Run `vagrant up` to turn on the devshop server.
4. Once the devmaster server is finished provisioning, put the aegir SSH public key into the "remote" server:
  1. `vagrant ssh` to log into the devmaster server.
  2. `sudo cat /var/aegir/.ssh/id_rsa.pub` to output the server's public key.  Copy this.
  3. `exit` to exit the devmaster server.
  4. `vagrant ssh` to log into the remote server.
  5. `sudo su -` to switch to root user.
  6. `vi ~/.ssh/authorized_keys` to edit the SSH authorized keys. Paste in the devmaster servers public key and save the file.
  7. `exit` the remote server.
  8. `vagrant ssh` back into the devmaster server.
  9. `sudo su - aegir` to switch to aegir user.
  10. `devshop remote:install` to run the remote server install script.
5. At the end, you will be shown the MySQL username and password, and the apache restart command. Use these to create a server in the devmaster front-end.