.hooks
======

This module complies with the soon-to-be-created "dotHooks" standard.

Enable this module to allow your own scripts to run hooks while in devshop, and hopefully more in the future.

How to use
----------

Add a file called `.hooks` to your project's git repository.

Then, add a "deploy" hook to it:

```
deploy: |
  drush updb -y
  drush cc all
  drush fra -y
```

Available Hooks
---------------

`verify`

Runs on devshop "verify" task.

`deploy`

Runs on "devshop-deploy" task. This is what happens everytime you push code to your git repository or change branches or tags.