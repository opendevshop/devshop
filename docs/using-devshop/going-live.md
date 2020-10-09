# Going Live

## Adding Domains

To take your site "live", you will want to add your own custom domain to an environment.

In aegir these are called "Domain Aliases".

In devshop, you can easily add additional _Domain Aliases_ via the _Environment Settings_ form.

1. Visit the Project Dashboard for the project you wish to launch.
2. If you don't have a "Live" environment yet, create one. Call it "live" or "production" or some other name that indicates that it is the live site.
3. Visit the Project Settings and select the environment you've just created as the **Live Environment**.
4. Click the _Environment Settings_ icon ![Push this button to open Environment Settings.](../.gitbook/assets/settings.png) icon on your live environment and select _Environment Settings_.
5. Check _Lock Database_ to prevent anyone from destroying your live database.
6. Scroll down and add "Domain Aliases" for your live domain. Add both "yourdomain.com" and "www.yourdomain.com", then select one of them as the _Redirect_ target. \(using "www" as your main URL is recommended.\)
7. Hit _Save_ and wait for the environment verification to complete.
8. Add DNS A records to point to your devshop server for all the domain aliases you added:

   ```text
    mydomain.com. 1800 IN A 1.2.3.4
    www.mydomain.com. 1800 IN A 1.2.3.4
   ```

Once the DNS resolves to your server, you should be able to see your website!

