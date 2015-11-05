core = 7.x
api = 2

projects[drupal][type] = "core"
projects[drupal][version] = "7.41"

defaults[projects][subdir] = "contrib"
defaults[projects][type] = "module"

; this makefile will make sure we get the development code from the
; aegir modules instead of the tagged releases
includes[devshop] = "drupal-org.make"

; Aegir Modules
; For development, use latest branch.
; For release, use tagged version
projects[hosting][version] = "3.1"

; Aegir Core not included in hosting.module
projects[eldir][type] = theme
projects[hosting_git][subdir] = aegir
projects[hosting_remote_import][subdir] = aegir
projects[hosting_site_backup_manager][subdir] = aegir
projects[hosting_tasks_extra][subdir] = aegir

; Contrib Modules
projects[betterlogin][version] = 1.4
projects[entity][version] = 1.6
projects[openidadmin][version] = 1.0
projects[overlay_paths][version] = 1.3
projects[r4032login][version] = 1.8
projects[admin_menu][version] = "3.0-rc5"
projects[adminrole][version] = "1.0"
projects[jquery_update][version] = "2.7"
projects[views][version] = "3.12"
projects[views_bulk_operations][version] = "3.3"
projects[ctools][version] = "1.9"
projects[features][version] = "2.7"
projects[hosting_filemanager][version] = "1"

; Aegir Contrib maintained by devshop maintainers
;projects[hosting_solr][version] = "1"
projects[hosting_logs][version] = "3.0-beta1"
projects[aegir_ssh][version] = "0"
projects[sshkey][version] = "2"

