# Using SSL

DevShop & Aegir make setting up SSL relatively easy.

## Software Packages

The required packages are now installed via the devshop install script and Ansible Playbooks. 

## Manual Package Installation

If you are missing these packages, you can install them:

On debian systems, only the `openssl` package is required:

    $ sudo apt-get install openssl

On RedHat systems, you also need the `mod_ssl` package.

    $ yum install openssl mod_ssl

## Aegir SSL Setup

There are more steps you need to take in the DevShop/Aegir front-end to enable SSL

1. Enable "Hosting SSL" Module:
    - Visit **Admin > Hosting**.
    - Check the box for **SSL Support** (if you are using Apache) or **NGINX SSL Support** (if you are using NGINX). Press the **Save configuration** button.

2. Configure your server to use *Apache SSL* or *NGINX SSL*:
    - Visit **Servers**.
    - Click the server that is hosting the site you wish to apply SSL to.
    - Click **Edit**.
    - Select *apache_ssl* or *nginx_ssl* from **Web**.
    - Click **Save**.
    - A *Verify* task for the server is queued.
    - Once the Server's *Verify* task is complete, a *Verify* task will be queued for every environment hosted on that server.

3. Configure your site to use SSL:
    - Visit the project dashboard for your project.
    - Click the *Environment Settings*  icon (![Environment Settings button.](images/settings.png "Push this button to open Environment Settings.")) on the environment you wish to add SSL to.
    - Click **Environment Settings**
    - Scroll down to **Encryption**. 
    - Select *Enable* if you want https to be optional.
    - Select *Required* if you want users to always be redirected to https.
    - Scroll down and click **Save**.
    - Wait for the site to be verified.
    - Visit the environment's URL and you should be able to access it via HTTPS.
    
## Adding Commercial SSL

If you wish to use your own commercial certificate and key you will need to do the following:

1. Follow the directions above, using the "Generate new encryption key" option and using your site's domain name for the "New encryption key". 

    - This will create a site directory under /var/aegir/config/ssl.d/example.com. With this step, you have created a self-signed certificate, and your site is now configured to use it.
    - This generated a 2048 bit RSA key for you along with a CSR (Certificate Signing Request). If you prefer to generate your own RSA key, replace the files (openssl.key and openssl.csr) in the /var/aegir/config/ssl.d/example.com directory with your RSA key and associated CSR.

2. Copy and paste the .csr file into the form for the issuing Certificate Authority (CA) to create your certificate.

    - When your certificate has been generated, download the files from the issuing authority and place in your temporary folder on your PC. You may have more than one .crt files, in this case you have a "bundle" or what we call a "certificate chain" that you need to add in aegir (see below).
    - Transfer all the files to /var/aegir/config/ssl.d/example.com. Rename the site .crt file to openssl.crt. If you have a certificate chain, install it in openssl_chain.crt. You should have at least three files in the directory (openssl.crt, openssl.key, openssl.csr, and optionally openssl_chain.crt).

3. Click the environment's *Environment Settings* icon ![Environment Settings button.](images/settings.png "Push this button to open Environment Settings.") icon and select *Verify*.

You should now be able to access your site via https:// using your commercial certificate.


## NGINX Certificates

It is recommended to allow Aegir to create a default self-signed certificate and key first, and then replace the contents of both files (not the files itself) with your real key and certificate. Any chained certificates (bundles) should be included in the same file, directly below your own certificate - there is no need for extra files/lines like it is for Apache configuration.

## Enabling SSL on the DevShop Dashboard

Once you have configured your server for SSL, you might want to enable HTTPS for your DevShop Dashboard website as well.

To enable HTTPS on your Aegir/DevShop front-end, find the "hostmaster" site.

The hostmaster site node is always available at the path alias "hosting/c/hostmaster", for example http://devshop.local.computer/hosting/c/hostmaster

You can also find the hostmaster site node by visiting http://devshop.local.computer/hosting/sites. Look for the site with the **OpenDevShop DevMaster** profile.

1. Visit the hostmaster site node page. http://devshop.local.computer/hosting/c/hostmaster
2. Click *Edit*.
3. Select the *SSL* fieldset.
4. Select *Optional* or *Required* for **Encryption**.
5. Enter a name for your *New Encryption Key*. Let Aegir generate a self-signed key first. 
5. Press the **Save** button.

Once the site verifies, you should be able to access your site via HTTPS.