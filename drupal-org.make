core = 7.x
api = 2

defaults[projects][subdir] = "contrib"
defaults[projects][type] = "module"

; Update this with each new release of devshop
projects[devshop_stats][version] = 1.x
projects[devshop_stats][subdir] = "contrib"

; Aegir Modules
; For development, use latest branch.
; For release, use tagged version
projects[hosting][subdir] = aegir
projects[hosting][version] = "3.161"

; Aegir Core not included in hosting.module
projects[eldir][type] = theme
projects[eldir][version] = "3.160"

projects[hosting_git][subdir] = aegir
projects[hosting_git][version] = "3.162"

projects[hosting_https][subdir] = aegir
projects[hosting_https][version] = "3.160"
projects[hosting_https][patch][] = "https://www.drupal.org/files/issues/2018-11-17/3014468-graceful-fail_0.patch"

projects[hosting_remote_import][subdir] = aegir
projects[hosting_remote_import][version] = "3.160"

projects[hosting_site_backup_manager][subdir] = aegir
projects[hosting_site_backup_manager][version] = "3.160"

projects[hosting_tasks_extra][subdir] = aegir
projects[hosting_tasks_extra][version] = "3.160"

projects[hosting_logs][subdir] = aegir
projects[hosting_logs][version] = "3.160"

projects[hosting_filemanager][subdir] = aegir
projects[hosting_filemanager][version] = "1.x"

projects[aegir_ssh][subdir] = aegir
projects[aegir_ssh][version] = 1.0

projects[aegir_config][subdir] = aegir
projects[aegir_config][version] = 1.00-beta1

; Not working yet.
;projects[hosting_solr][version] = "1"

; Contrib Modules
projects[sshkey][version] = "2.0"
projects[betterlogin][version] = 1.5
projects[entity][version] = 1.9
projects[openidadmin][version] = 1.0
projects[overlay_paths][version] = 1.3
projects[r4032login][version] = 1.8
projects[admin_menu][version] = "3.0-rc5"
projects[adminrole][version] = "1.1"
projects[jquery_update][version] = "3.0-alpha5"
projects[views][version] = "3.20"
projects[views_bulk_operations][version] = "3.5"
projects[ctools][version] = "1.14"
projects[features][version] = "2.10"
projects[distro_update][version] = "1"
projects[module_filter][version] = "2"
projects[libraries][version] = 2.5
projects[cas][version] = 1.7
projects[cas_attributes][version] = 1.x
; projects[hybridauth][version] = 2.15
projects[composer_autoloader][version] = 1.3

; Dehydrated for LetsEncrypt.org
libraries[dehydrated][download][type] = git
libraries[dehydrated][download][url] = https://github.com/lukas2511/dehydrated.git
libraries[dehydrated][destination] = modules/aegir/hosting_https/submodules/letsencrypt/drush/bin

; PHPCAS
;libraries[cas][download][type] = "git"
;libraries[cas][download][url] = "https://github.com/apereo/phpCAS"
;libraries[cas][destination] = "libraries"

; Hybrid Auth
;libraries[hybridauth][download][type] = "git"
;libraries[hybridauth][download][url] = "https://github.com/hybridauth/hybridauth"
;libraries[hybridauth][download][tag] = "v2.10.0"
;libraries[hybridauth][destination] = "libraries"

; Timeago module
projects[timeago][version] = 2.3

; JQuery TimeAgo plugin
libraries[timeago][download][type] = get
libraries[timeago][download][url] = https://raw.githubusercontent.com/rmm5t/jquery-timeago/v1.5.3/jquery.timeago.js
libraries[timeago][destination] = libraries

; Bootstrap base theme
projects[bootstrap][type] = theme
projects[bootstrap][version] = 3.21

;projects[intercomio][type] = module
;projects[intercomio][download][type] = git
;projects[intercomio][download][branch] = composer-autoload
;projects[intercomio][download][url] = "https://github.com/thinkdrop/drupal-intercomio.git"
projects[intercomio][version] = 1.x
