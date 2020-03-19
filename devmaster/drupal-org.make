core = 7.x
api = 2

defaults[projects][subdir] = "contrib"
defaults[projects][type] = "module"

; Update this with each new release of devshop
projects[devshop_stats][version] = 1.x

; Aegir Modules
projects[hosting][version] = "3.180"

; Aegir Core not included in hosting.module
projects[eldir][type] = theme
projects[eldir][version] = "3.180"

projects[hosting_git][version] = "3.181"

projects[hosting_https][version] = "3.182"

projects[hosting_remote_import][version] = "3.180"

projects[hosting_site_backup_manager][version] = "3.180"

projects[hosting_tasks_extra][version] = "3.180"

projects[hosting_logs][version] = "3.181"

projects[hosting_filemanager][version] = "1.x"

projects[aegir_ssh][version] = "1.0"

projects[aegir_config][version] = "1.x"

projects[aegir_ansible][version] = "1.x"

projects[aegir_cloud][version] = "1.x"

; Not working yet.
;projects[hosting_solr][version] = "1"

; Contrib Modules
projects[devel][version] = "1.7"
projects[sshkey][version] = "2.0"
projects[betterlogin][version] = 1.5
projects[composer_manager][version] = 1.8
projects[entity][version] = 1.9
projects[openidadmin][version] = 1.0
projects[overlay_paths][version] = 1.3
projects[r4032login][version] = 1.8
projects[admin_menu][version] = "3.0-rc6"
projects[adminrole][version] = "1.1"
projects[jquery_update][version] = "3.0-alpha5"
projects[views][version] = "3.23"
projects[views_bulk_operations][version] = "3.5"
projects[ctools][version] = "1.15"
projects[features][version] = "2.11"
projects[distro_update][version] = "1"
projects[module_filter][version] = "2.2"
projects[libraries][version] = 2.5
projects[token][version] = 1.7
; projects[hybridauth][version] = 2.15
projects[statsd][version] = 1.1
projects[hosting_statsd][version] = 1.0-beta1
projects[intercomio][version] = 1.0-beta2
projects[navbar][version] = 1.7
projects[chosen][version] = 2.1

projects[cas][version] = 1.7
projects[cas][patch][] = "https://www.drupal.org/files/issues/2018-12-13/3020349-cas-library-path.patch"
projects[cas_attributes][version] = 1.0-rc3

; Bootstrap base theme
projects[bootstrap][type] = theme
projects[bootstrap][version] = "3.26"

; Timeago module
projects[timeago][version] = 2.3

; JQuery TimeAgo plugin
libraries[timeago][download][type] = get
libraries[timeago][download][url] = https://raw.githubusercontent.com/rmm5t/jquery-timeago/v1.5.3/jquery.timeago.js
libraries[timeago][destination] = libraries

; @TODO: Uncomment once it is in the whitelist: https://www.drupal.org/project/drupalorg_whitelist/issues/3024898
; Library: Modernizr
; libraries[modernizr][download][type] = git
; libraries[modernizr][download][url] = https://github.com/BrianGilbert/modernizer-navbar.git
; libraries[modernizr][download][revision] = 5b89d9225320e88588f1cdc43b8b1e373fa4c60f

; Library: Backbone
libraries[backbone][download][type] = git
libraries[backbone][download][url] = https://github.com/jashkenas/backbone.git
libraries[backbone][download][tag] = 1.0.0

; Library: Underscore
libraries[underscore][download][type] = git
libraries[underscore][download][url] = https://github.com/jashkenas/underscore.git
libraries[underscore][download][tag] = 1.5.0

; Library: Chosen
libraries[chosen][download][type] = "git"
libraries[chosen][download][url] = "https://github.com/harvesthq/chosen-package.git"
