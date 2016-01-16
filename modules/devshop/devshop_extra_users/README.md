DevShop Extras: Users
=====================

This module provides an example of how to use the DevShop front-end to take action
after an environment is installed.

Enable it, then visit the "Create Environment" form for a project. 

You will see a form field for "Manager Email".  This field gets saved into the environment settings automatically.

Then, in `devshop_extras_users.drush.inc` during the `hook_post_hosting_TASKTYPE_task`
hook, this module creates a new user in your drupal site immediately after the 
installation.

Nothing else happens here.  Typically you would want to set a role or send an 
email. 

Use this code as an example for extending your own environments.