# Deployment Hooks

DevShop has many ways to allow customized actions after deployment of code and data.

## Project Settings: Default Deploy Hooks

In the project settings form, you will see **Default Deploy Hooks** section with the following options:

* Run database updates
* Clear all caches
* Revert all features

If you have the **DevShop dotHooks** module enabled, you will also see:

* Run deploy commands in the .hooks file.

If you have the **DevShop Acquia** module enabled, you will also see:

* Run Acquia Cloud Hooks.

You will also see a checkbox for "Allow environment-specific deploy hook configuration.".

## Environment Settings: Deployment Hooks

In each Environment's Settings form, you will see a section called "Deployment Hooks" if "Allow environment-specific deploy hook configuration." was checked in the _Project Settings_ form.

## DevShop dotHooks

DevShop now supports placing your hook commands in a file called `.hooks`, `.hooks.yml`, or `.hooks.yaml`.

Create a file with the format below to give your developers more control over what happens when new code is deployed.

System environment variables are available.

```text
# Fires after an environment is installed.
install: |
  drush {{alias}} vset site_name "Hooks Hooks Hooks"

# Fires after code is deployed. A "deployment" happens when you push to your
# git repository or select a new branch or tag for your environment.
deploy: |
  echo "Running hooks in the $DEVSHOP_ENVIRONMENT environment for the $DEVSHOP_PROJECT..."
  drush {{alias}} updb -y
  drush {{alias}} cc all
  echo "Environment Variables:"
  env

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

## DevShop Aquia Cloud Hooks

DevShop now supports Acquia Cloud hooks. If your project is from Acquia, and you use the Cloud Hooks feature, you can now configure your project and environments to use them as deploy hooks when hosting sites in OpenDevShop.

To use DevShop Acquia Cloud Hooks integration:

1. Visit Admin &gt; Hosting &gt; Features.
2. Check "DevShop Acquia" under the "Experimental" category and submit the form.
3. Visit your project's settings page.
4. Under "Deploy Hooks", check the box for "Use Acquia Cloud Hooks" and submit the form.
5. Create your cloud hooks: Visit [https://github.com/acquia/cloud-hooks.git](https://github.com/acquia/cloud-hooks.git) for more information.

