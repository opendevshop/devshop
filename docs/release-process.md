Release Process
===============

There's a lot of moving parts so creating a new release is not simple.

Follow this guide.

1. Create new releases for contrib projects, if necessary.
  - hosting_solr
  - hosting_logs
  - etc
4. Update `opendevshop/devmaster/devmaster.make` with the latest version of drupal core.
3. Update `opendevshop/devmaster/devmaster.make` with the latest releases of contrib modules.
4. Update `opendevshop/devshop/vars.yml` with the latest versions of drush modules.
5. Using the commit log as your guide, add a list of new features to the CHANGELOG.org in the devshop repo.  Use the "x commits since this release" feature on github's releases page.  Include changes in both devshop and devmaster repos in the changelog.
7. Update `opendevshop/devshop/README.md` "Versions" section to match the latest.
2. Create a new release branch for devmaster and devshop repos: `release-x.y.z`
6. Edit the release version in the `devshop` repo in the files `build-devmaster.make`, `install.sh`, and `vars.yml`. This can be done in the release branch, it won't get merged to 0.x. 
5. Create and push a new tag for `devmaster` and for `devshop`: `x.y.z`
6. Create a new "release" on github to match the new tag at https://github.com/opendevshop/devshop/releases/new.  Copy the release notes from CHANGELOG.md into 
7. Edit http://drupal.org/project/devshop to show the latest release.
7. Announce!