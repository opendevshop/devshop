Acquia Cloud Hooks Integration
==============================

DevShop now supports firing Acquia Cloud Hooks when deploying to environments.

The code in acquia.drush.inc will detect an acquia repo and trigger your hooks
to run.

Currently, only the `post-code-update` hook is supported.  More to come.