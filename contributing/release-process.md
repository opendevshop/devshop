# Release Process

There's a lot of moving parts so creating a new release is not simple.

Follow this guide.

1. Create new releases for contrib projects, if necessary.
   * hosting\_solr
   * hosting\_logs
   * etc
2. Create a new release of "devshop\_stats" project:

   Rewrite the release number to match drupal.org standards. Only release a new stats module for each minor release.

   * cd ./source/devmaster-1.x/profiles/modules/contrib/devshop\_stats
   * git tag 7.x-1.0-beta4
   * git push 7.x-1.0-beta4
   * Visit module page: [https://www.drupal.org/project/devshop\_stats/releases](https://www.drupal.org/project/devshop_stats/releases).
   * Click "Add New Release".
   * Select the tag you created.

3. Update `opendevshop/devmaster/devmaster.make` with the latest version of drupal core.
4. Update `opendevshop/devmaster/devmaster.make` with the latest releases of contrib modules.
5. Update `opendevshop/devshop/vars.yml` with the latest versions of drush modules.
6. Using the commit log as your guide, add a list of new features to the CHANGELOG.org in the devshop repo.  Use the "x commits since this release" feature on github's releases page.  Include changes in both devshop and devmaster repos in the changelog.
7. Update `opendevshop/devshop/docs/install.md` with the latest version in the example install script.
8. Create a new release branches for devmaster, devshop, and devshop\_provision repos using the `release-prep.sh` script. This script takes a version for an argument and creates release branches in the three repos.
9. In your release branch, edit the version in the files:
   * `build-devmaster.make`
   * `install.sh`
   * `vars.yml`
   * `opendevshop/devmaster/devmaster.make` \(Edit the devshop\_stats version.\)
   * `opendevshop/devmaster/VERSION.txt`
10. Create and push a new tag for `devmaster` & `devshop` using the `release.sh` script.  This script takes a version for an argument and creates release tags in the three repos.
11. Create a new "release" on github to match the new tag at [https://github.com/opendevshop/devshop/releases/new](https://github.com/opendevshop/devshop/releases/new).  Copy the release notes from CHANGELOG.md into 
12. Edit [http://drupal.org/project/devshop](http://drupal.org/project/devshop) to show the latest release.
13. Update the "Current Version" displayed on the gh-pages branch. [https://github.com/opendevshop/devshop/edit/gh-pages/index.html](https://github.com/opendevshop/devshop/edit/gh-pages/index.html) 
14. Announce!

