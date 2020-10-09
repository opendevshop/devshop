DevShop Deploy Component
===============================

The `devshop/deploy` component lets Composer-based app developers define 
what happens to their code during deployment across environments, and allows 
them to control what stages are run in each environment.

The component defines a standard set of named "stages" that code 
goes through during the development life cycle. Each stage is a list of commands 
that can be pre-defined for standard projects like Drupal or Wordpress, or 
defined in a per-project or per-environment basis.

Finally, the component provides a command called `deploy` that runs the desired 
stages for that environment.

## Stages

1. `git`: The commands to clone the source code (or ensure it's already there) and check out the desired version.
2. `build`: The commands to run to prepare the source code, such as `composer install` or `npm install`.
3. `install`: The commands to run to prepare your application once all code and services are ready, such as `drush site:install` or `drush site:import ~/backups/site.sql`. (Normally skipped in "prod" environments.) 
4. `deploy`: The commands to run immediately after new code and services are ready, such as `drush updb -y && drush cr all`  
5. `Test`: The commands to test a running site.  

Resources
---------

  * [Documentation](https://github.com/opendevshop/devshop/blob/1.x/README.md)
  * [Contributing](https://github.com/opendevshop/devshop/blob/1.x/CONTRIBUTING.md)
  * [Report issues](https://github.com/opendevshop/devshop/issues) and
    [send Pull Requests](https://github.com/opendevshop/devshop/pulls)
    in the [main DevShop repository](https://github.com/opendevshop/devshop)

Credits
-------

Jon Pugh 
