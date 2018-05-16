# Acquia Cloud Hooks Integration

## Acquia Cloud Hooks Integration

DevShop now supports firing Acquia Cloud Hooks when deploying to environments.

The code in acquia.drush.inc will detect an acquia repo and trigger your hooks to run.

See the Acquia Cloud Hooks repository for more information and samples: [https://github.com/acquia/cloud-hooks](https://github.com/acquia/cloud-hooks)

Currently, only the `post-code-update` hook is supported. More to come.

## Hooks

### post-code-deploy

Changing branch or tag manually.

DevShop Equivalent: provision-devshop-deploy when triggered from front-end.

```text
post-code-deploy site target-env source-branch deployed-tag repo-url repo-type
```

### post-code-update

Deploying code to a branch environment automatically.

DevShop Equivalent: provision-devshop-deploy

```text
post-code-deploy site target-env source-branch deployed-tag repo-url repo-type
```

### post-db-copy

_\*Coming Soon..._

```text
post-db-copy site target-env db-name source-env
```

### post-files-copy

_\*Coming Soon..._

```text
post-files-copy mysite prod dev
```

