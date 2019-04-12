# Using settings.php

## Background

### "Sites Folder"

Aegir was designed around multi-site. Because of this, it always uses the domain-specific "sites folder" for settings, files and backups.

```text
    sites/dev.myproject.mydevshopserver.com/settings.php
    sites/dev.myproject.mydevshopserver.com/files
```

Be aware that the `sites/default/settings.php` file is _not used_ at all.

### Do not edit Settings.php.

Do not write changes directly to your site's `settings.php` file. It is written automatically.

## Settings.php includes

To allow users to make customizations to settings.php, aegir and devshop include certain files from elsewhere.

To see exactly what files are included, you can open a working settings.php file for a working environment.

Possible include files:

* sites/all/settings.devshop.php
* sites/default/settings.devshop.php
* sites/ENVIRONMENT.PROJECT.DEVMASTER-HOSTNAME/local.settings.php

```text
<?php

   # Include devshop environment configuration settings, if there is any.
    if (file_exists(__DIR__ . '/../all/settings.devshop.php')) {
      include_once(__DIR__ . '/../all/settings.devshop.php');
    }

    if (file_exists(__DIR__ . '/../default/settings.devshop.php')) {
      include_once(__DIR__ . '/../default/settings.devshop.php');
    }

    # Additional host wide configuration settings. Useful for safely specifying configuration settings.
    if (is_readable('/var/aegir/config/includes/global.inc')) {
      include_once('/var/aegir/config/includes/global.inc');
    }

    # Additional site configuration settings.
    if (is_readable('/var/aegir/devmaster-0.x/sites/ENVIRONMENT.PROJECT.DEVMASTER-HOSTNAME/local.settings.php')) {
      include_once('/var/aegir/devmaster-0.x/sites/ENVIRONMENT.PROJECT.DEVMASTER-HOSTNAME/local.settings.php');
    }

?>
```

### DevShop Project and Environment Variables

A server environment variable is written in settings.php. You can check them if you need environment-specific settings.
  
  ```php
  # This site's devshop project name and environment name.
  $_SERVER['DEVSHOP_ENVIRONMENT'] = 'production';
  $_SERVER['DEVSHOP_PROJECT'] = 'thinkdrop';
  ```

For example, you can disable all caches in all environments except for production:

    ```php
    if ($_SERVER['DEVSHOP_ENVIRONMENT'] != 'production') {
        $conf['cache'] = 0;                       // Page cache
        $conf['page_cache_maximum_age'] =  0;     // External cache TTL
        $conf['preprocess_css'] = FALSE;          // Optimize css
        $conf['preprocess_js'] = FALSE;           // Optimize javascript
        $conf['views_skip_cache'] = TRUE;         // Views caching
     }

### Remote Servers

If you are using Remote Servers to host your sites, please be aware of the Verify/RSYNC System.

All code is stored on the server\_master, and is copied to remote servers via RSYNC during the `provision-verify` command.

This Rsync will also _delete_ files if they are removed from server\_master.

When modifying settings.php using these includes, do so on the server\_master server \(or in your git repository.\). Then, **Verify** the environment. The **Verify** task will copy the custom settings to remote server.

