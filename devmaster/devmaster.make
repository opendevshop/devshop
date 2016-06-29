core = 7.x
api = 2

projects[drupal][type] = "core"
projects[drupal][version] = "7.44"

defaults[projects][subdir] = "contrib"
defaults[projects][type] = "module"

# Update this with each new release of devshop
projects[devshop_stats][version] = "1.x"
projects[devshop_stats][subdir] = "contrib"

; this makefile will make sure we get the development code from the
; aegir modules instead of the tagged releases
includes[devshop] = "drupal-org.make"

; Aegir Modules
; For development, use latest branch.
; For release, use tagged version
projects[hosting][version] = "3.6"
projects[hosting][subdir] = "aegir"

; Aegir Core not included in hosting.module
projects[eldir][type] = theme

projects[hosting_git][version] = "3.5"
projects[hosting_git][subdir] = aegir

projects[hosting_remote_import][subdir] = aegir

projects[hosting_site_backup_manager][subdir] = aegir
projects[hosting_site_backup_manager][version] = "3.5"

projects[hosting_tasks_extra][subdir] = aegir
projects[hosting_tasks_extra][version] = "3.5"

projects[hosting_filemanager][subdir] = aegir
projects[hosting_logs][subdir] = aegir
projects[hosting_logs][version] = 3.1
projects[aegir_ssh][subdir] = aegir
projects[aegir_ssh][version] = 0.3
projects[aegir_config][subdir] = aegir
projects[aegir_config][version] = 1.x
projects[aegir_ansible][subdir] = aegir
projects[aegir_ansible][version] = 1.x
projects[aegir_cloud][subdir] = aegir
projects[aegir_cloud][version] = 1.x

; Not working yet.
;projects[hosting_solr][version] = "1"

; Contrib Modules
projects[sshkey][version] = "2.0"
projects[betterlogin][version] = 1.4
projects[entity][version] = 1.7
projects[openidadmin][version] = 1.0
projects[overlay_paths][version] = 1.3
projects[r4032login][version] = 1.8
projects[admin_menu][version] = "3.0-rc5"
projects[adminrole][version] = "1.1"
projects[jquery_update][version] = "3.0-alpha3"
projects[views][version] = "3.14"
projects[views_bulk_operations][version] = "3.3"
projects[ctools][version] = "1.9"
projects[features][version] = "2.10"
projects[distro_update][version] = "1"
projects[module_filter][version] = "2"
projects[intercomio][version] = "1"

; Bootstrap base theme
projects[bootstrap][type] = theme
projects[bootstrap][version] = 3.5
