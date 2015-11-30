Deployment Hooks
================

DevShop has many ways to allow customized actions after deployment of code and data.

Project Settings: Default Deploy Hooks
--------------------------------------

In the project settings form, you will see **Default Deploy Hooks** section with the following options:

- Run database updates
- Clear all caches
- Revert all features

You will also see a checkbox for "Allow environment-specific deploy hook configuration.".

Environment Settings: Deployment Hooks
--------------------------------------

In each Environment's Settings form, you will see a section called "Deployment 
Hooks" if "Allow environment-specific deploy hook configuration." was checked 
in the *Project Settings* form.

DevShop .Hooks
--------------

DevShop now supports placing your hook commands in a file called `.hooks`, `.hooks.yml`, or `.hooks.yaml`.

Create a file with the format below to give your developers more control over what happens 
when new code is deployed.

You must enable to "DevShop .Hooks" module to get this functionality. 

It will be enabled by default in the next release.

```
# Fires after an environment is installed.
install: |
  drush {{alias}} vset site_name "Hooks Hooks Hooks"

# Fires after code is deployed. A "deployment" happens when you push to your
# git repository or select a new branch or tag for your environment.
deploy: |
  drush {{alias}} updb -y
  drush {{alias}} cc all

# Fires after "verify" task.
verify: |
  drush {{alias}} status

# Fires after "Run Tests" task.
test: |
  drush {{alias}} uli


# Fires after "Deploy Data (Sync)" task.
sync: |
  drush {{alias}} en devel -y
```