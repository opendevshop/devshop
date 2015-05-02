Release Process
===============

There's a lot of moving parts so creating a new release is not simple.

Follow this guide.

1. Create new releases for contrib projects, if necessary.
  - hosting_solr
  - hosting_logs
  - etc
2. Update `opendevshop/devmaster/devmaster.make` with the latest releases of contrib modules.
3. Commit, push, and test the new makefile
4. Create and push a new tag for `devmaster`: 6.x-0.1, 6.x-0.2, etc.
5. Create a new release for `devmaster` on drupal.org.

 