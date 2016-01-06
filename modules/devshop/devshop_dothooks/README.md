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
  drush {{alias}} updb -y
  drush {{alias}} cc all
  drush {{alias}} fra -y
```

You can also use the array syntax to specify commands:

```
deploy:
  - drush {{alias}} updb -y
  - drush {{alias}} cc all
  - drush {{alias}} fra -y
```

You must use {{alias}} when calling drush to ensure you are controlling the correct site.

Available Hooks
---------------

`verify`

Runs on devshop "verify" task.

`deploy`

Runs on "devshop-deploy" task. This is what happens everytime you push code to your git repository or change branches or tags.