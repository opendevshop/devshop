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
    
Adding Commercial SSL
---------------------

If you wish to use your own commercial certificate and key you will need to do the following:

1. Follow the directions above, using the "Generate new encryption key" option and using your site's domain name for the "New encryption key". 

    - This will create a site directory under /var/aegir/config/ssl.d/example.com. With this step, you have created a self-signed certificate, and your site is now configured to use it.
    - This generated a 2048 bit RSA key for you along with a CSR (Certificate Signing Request). If you prefer to generate your own RSA key, replace the files (openssl.key and openssl.csr) in the /var/aegir/config/ssl.d/example.com directory with your RSA key and associated CSR.

2. Copy and paste the .csr file into the form for the issuing Certificate Authority (CA) to create your certificate.

    - When your certificate has been generated, download the files from the issuing authority and place in your temporary folder on your PC. You may have more than one .crt files, in this case you have a "bundle" or what we call a "certificate chain" that you need to add in aegir (see below).
    - Transfer all the files to /var/aegir/config/ssl.d/example.com. Rename the site .crt file to openssl.crt. If you have a certificate chain, install it in openssl_chain.crt. You should have at least three files in the directory (openssl.crt, openssl.key, openssl.csr, and optionally openssl_chain.crt).

3. Click the environment's *Gear* icon and select *Verify*.

You should now be able to access your site via https:// using your commercial certificate.


NGINX Certificates
------------------

It is recommended to allow Aegir to create a default self-signed certificate and key first, and then replace the contents of both files (not the files itself) with your real key and certificate. Any chained certificates (bundles) should be included in the same file, directly below your own certificate - there is no need for extra files/lines like it is for Apache configuration.