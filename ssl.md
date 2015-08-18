Using SSL
=========

DevShop & Aegir make setting up SSL relatively easy.

Software Packages
-----------------

The `openssl` packages are now installed via the devshop installer.

However this is a recent addition, you may need to install them yourself.

Package Installation
--------------------

If you are missing these packages, you can install them:

On debian systems, only the `openssl` package is required:

    $ sudo apt-get install openssl

On RedHat systems, you also need the `mod_ssl` package.

    $ yum install openssl mod_ssl

SSL Setup
---------

There are a few steps you need to take in the devshop front-end to get SSL going on an environment.

1. Enable "Hosting SSL" Module:
  - Visit **Admin > Hosting**.
  - Check the box for **SSL Support** or **NGINX SSL Support** and hit **Save configuration**.

2. Configure your server to use *Apache SSL* or *NGINX SSL*:
  - Visit **Servers**.
  - Click the server that is hosting the site you wish to apply SSL to.
  - Click **Edit**.
  - Select *apache_ssl* or *nginx_ssl* from **Web**.
  - Click **Save**.

3. Configure your site to use SSL:
  - Visit the project dashboard for your project.
  - Click the *Gear* icon next to the environment you wish to add SSL to.
  - Click **Environment Settings**
  - Scroll down to **Encryption**. 
  - Select *Enable* if you want https to be optional.
  - Select *Required* if you want users to always be redirected to https.
  - Scroll down and click **Save**.
  - Wait for the site to be verified.
  - Visit the environment's URL and you should be able to access it via HTTPS.
  
