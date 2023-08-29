# Manual upgrade instructions for DevShop 1.7 and earlier.

The latest DevShop uses a Git codebase with Composer for the backend and front-end code.

The "hostmaster" site is now located here in the source repo at [../src/DevShop/Control](../src/DevShop/Control).

To upgrade manually, take these steps.

Your mileage may vary.

1. Checkout `/usr/share/devshop` to the version you want to install. See  https://github.com/opendevshop/devshop/releases for the latest.
2. Manually copy your hostmaster sites folder /var/aegir/devmaster-X/sites/your.devshop.com folder to the new devshop control root: `/usr/share/devshop/src/DevShop/Control/web/sites/your.devshop.com`.
3. Update hostmaster platform publish_path to the new root (/usr/share/devshop/src/DevShop/Control/web). Find the Platform ID in the web ui, or add a JOIN query.

        UPDATE {hosting_platform} SET publish_path = '/usr/share/devshop/src/DevShop/Control/web' WHERE nid = $YOUR_HOSTMASTER_PLATFORM_NID  

4. Run verify task on hostmaster to reconfigure drush aliases and apache configs to the new path.

        drush @hostmaster hosting-task hostmaster verify

5. Now that the site is pointing at the new code, run database update.

        drush @hostmaster updb


I


