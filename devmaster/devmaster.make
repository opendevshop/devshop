core = 7.x
api = 2

projects[drupal][type] = "core"
projects[drupal][version] = "7.41"

; this makefile will make sure we get the development code from the
; aegir modules instead of the tagged releases
includes[devshop] = "drupal-org.make"

; Aegir Modules
; For development, use latest branch.
; For release, use tagged version
projects[hosting][version] = "3.1"

; Contrib Modules
projects[admin_menu][version] = "3.0-rc5"
projects[admin_menu][subdir] = contrib

projects[adminrole][version] = "1.0"
projects[adminrole][subdir] = contrib

projects[jquery_update][version] = "2.7"
projects[jquery_update][subdir] = contrib

projects[views][version] = "3.12"
projects[views][subdir] = contrib

projects[views_bulk_operations][version] = "3.3"
projects[views_bulk_operations][subdir] = contrib

projects[ctools][version] = "1.9"
projects[ctools][subdir] = contrib

projects[features][version] = "2.7"
projects[features][subdir] = contrib

projects[hosting_filemanager][version] = "1"
projects[hosting_filemanager][subdir] = contrib

projects[hosting_tasks_extra][version] = "3.1"
projects[hosting_tasks_extra][subdir] = contrib

; Aegir Contrib maintained by devshop maintainers
;projects[hosting_solr][version] = "1"
;projects[hosting_solr][subdir] = contrib

projects[hosting_logs][version] = "3.0-beta1"
projects[hosting_logs][subdir] = contrib

projects[hosting_site_backup_manager][version] = "3.3"
projects[hosting_site_backup_manager][subdir] = contrib

projects[aegir_ssh][version] = "0"
projects[aegir_ssh][subdir] = contrib

projects[sshkey][version] = "2"
projects[sshkey][subdir] = contrib
