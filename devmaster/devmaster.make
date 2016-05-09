core = 7.x
api = 2

projects[drupal][type] = "core"
projects[drupal][version] = "7.43"

# Update this with each new release of devshop
projects[devshop_stats][version] = "1.0-alpha2"

defaults[projects][subdir] = "contrib"
defaults[projects][type] = "module"

; this makefile will make sure we get the development code from the
; aegir modules instead of the tagged releases
includes[devshop] = "drupal-org.make"

projects[hostmaster][type] = "profile"
projects[hostmaster][version] = "3.4"
projects[hostmaster][subdir] = ""
projects[hostmaster][version] = "3.4"

; Aegir Modules
; For development, use latest branch.
; For release, use tagged version
projects[hosting][version] = "3.4"
projects[hosting][subdir] = "aegir"
projects[hosting][patch][687750] = "https://www.drupal.org/files/issues/2610122-circular-dependencies.patch"
projects[hosting][patch][2707157] = "https://www.drupal.org/files/issues/2707157-hosting-site-nid-variable.patch"

; Aegir Core not included in hosting.module
projects[eldir][type] = theme
projects[hosting_git][subdir] = aegir
projects[hosting_remote_import][subdir] = aegir
projects[hosting_site_backup_manager][subdir] = aegir
projects[hosting_site_backup_manager][patch][2619074] = https://www.drupal.org/files/issues/2619074-remove-varget_2.patch

projects[hosting_tasks_extra][subdir] = aegir
projects[hosting_tasks_extra][patch][2707129] = https://www.drupal.org/files/issues/2707129-upgrade-drupal_0.patch
projects[hosting_filemanager][subdir] = aegir
projects[hosting_logs][subdir] = aegir
projects[hosting_logs][version] = 3.x
projects[aegir_ssh][subdir] = aegir
projects[aegir_ssh][version] = 0.3
projects[aegir_config][subdir] = aegir
projects[aegir_config][version] = 1.x

; Not working yet.
;projects[hosting_solr][version] = "1"

; Contrib Modules
projects[sshkey][version] = "2"
projects[betterlogin][version] = 1.4
projects[entity][version] = 1.7
projects[openidadmin][version] = 1.0
projects[overlay_paths][version] = 1.3
projects[r4032login][version] = 1.8
projects[admin_menu][version] = "3.0-rc5"
projects[adminrole][version] = "1.1"
projects[jquery_update][version] = "3.0-alpha3"
projects[views][version] = "3.13"
projects[views_bulk_operations][version] = "3.3"
projects[ctools][version] = "1.9"
projects[features][version] = "2.10"
projects[distro_update][version] = "1"
projects[module_filter][version] = "2"
projects[intercomio][version] = "1"
projects[devshop_stats][version] = "1.0-beta2"

; Bootstrap base theme
projects[bootstrap][type] = theme
projects[bootstrap][version] = 3.5
