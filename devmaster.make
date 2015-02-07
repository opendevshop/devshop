core = 6.x
api = 2

projects[drupal][type] = "core"

; this makefile will make sure we get the development code from the
; aegir modules instead of the tagged releases
includes[devshop] = "drupal-org.make"

; Aegir Modules
projects[hosting][version] = "2.1"
projects[hosting][subdir] = aegir

projects[hosting_platform_pathauto][version] = "2.1"
projects[hosting_platform_pathauto][subdir] = contrib

projects[admin_menu][version] = "1.8"
projects[admin_menu][subdir] = contrib

projects[openidadmin][version] = "1.2"
projects[openidadmin][subdir] = contrib

projects[install_profile_api][version] = "2.2"
projects[install_profile_api][subdir] = contrib

projects[jquery_ui][version] = "1.5"
projects[jquery_ui][subdir] = contrib

projects[jquery_update][version] = "2.0-alpha1"
projects[jquery_update][subdir] = contrib

projects[modalframe][version] = "1.9"
projects[modalframe][subdir] = contrib

projects[views][version] = "3.0"
projects[views][subdir] = contrib

projects[views_bulk_operations][version] = "1.16"
projects[views_bulk_operations][subdir] = contrib

projects[ctools][version] = "1.11"
projects[ctools][subdir] = contrib

; Libraries
libraries[jquery_ui][download][type] = "get"
libraries[jquery_ui][destination] = "modules/contrib/jquery_ui"
libraries[jquery_ui][download][url] = "http://jquery-ui.googlecode.com/files/jquery-ui-1.7.3.zip"
libraries[jquery_ui][directory_name] = "jquery.ui"

; DevShop modules
projects[devshop_hosting][subdir] = devshop
projects[devshop_hosting][download][type] = git
projects[devshop_hosting][download][branch] = 6.x-2.x

; Contrib modules
projects[adminrole][subdir] = contrib
projects[ctools][subdir] = contrib
projects[jquery_update][subdir] = contrib

; Aegir Contrib
projects[hosting_filemanager][subdir] = contrib
projects[hosting_tasks_extra][subdir] = contrib

; Aegir Contrib maintained by devshop maintainers
projects[hosting_solr][subdir] = contrib
projects[hosting_solr][download][type] = git
projects[hosting_solr][download][branch] = 6.x-1.x

projects[hosting_drush_aliases][subdir] = contrib
projects[hosting_drush_aliases][download][type] = git
projects[hosting_drush_aliases][download][branch] = 6.x-1.x

projects[hosting_logs][subdir] = contrib
projects[hosting_logs][download][type] = git
projects[hosting_logs][download][branch] = 6.x-1.x

projects[hosting_site_backup_manager][subdir] = contrib
projects[hosting_site_backup_manager][download][type] = git
projects[hosting_site_backup_manager][download][branch] = 6.x-2.x
