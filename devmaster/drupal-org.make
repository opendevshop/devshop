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
projects[hosting][version] = "3.120"

; Aegir Core not included in hosting.module
projects[eldir][type] = theme
projects[eldir][version] = "3.120"

projects[hosting_git][subdir] = aegir
projects[hosting_git][version] = "3.120"

projects[hosting_remote_import][subdir] = aegir
projects[hosting_remote_import][version] = "3.120"

projects[hosting_site_backup_manager][subdir] = aegir
projects[hosting_site_backup_manager][version] = "3.120"

projects[hosting_tasks_extra][subdir] = aegir
projects[hosting_tasks_extra][version] = "3.120"

projects[hosting_logs][subdir] = aegir
projects[hosting_logs][version] = "3.120"

projects[hosting_filemanager][subdir] = aegir
projects[hosting_filemanager][version] = "1.x"

projects[aegir_ssh][subdir] = aegir
projects[aegir_ssh][version] = 1.0

projects[aegir_config][subdir] = aegir
projects[aegir_config][version] = 1.00-beta1

projects[aegir_ansible][subdir] = aegir
projects[aegir_ansible][version] = 1.00-beta1

projects[aegir_cloud][subdir] = aegir
projects[aegir_cloud][version] = 1.00-beta1

; Not working yet.
;projects[hosting_solr][version] = "1"

; Contrib Modules
projects[sshkey][version] = "2.0"
projects[betterlogin][version] = 1.4
projects[entity][version] = 1.8
projects[openidadmin][version] = 1.0
projects[overlay_paths][version] = 1.3
projects[r4032login][version] = 1.8
projects[admin_menu][version] = "3.0-rc5"
projects[adminrole][version] = "1.1"
projects[jquery_update][version] = "3.0-alpha5"
projects[views][version] = "3.18"
projects[views_bulk_operations][version] = "3.4"
projects[ctools][version] = "1.12"
projects[features][version] = "2.10"
projects[distro_update][version] = "1"
projects[module_filter][version] = "2"
projects[intercomio][version] = "1"
projects[libraries][version] = 2.3

; Hosting HTTPS with Let's Encrypt!
; @TODO: Update with a new alpha.
;projects[hosting_https][type] = module
;projects[hosting_https][download][type] = git
;projects[hosting_https][download][url] = https://gitlab.com/aegir/hosting_https.git
;projects[hosting_https][download][branch] = master
;projects[hosting_https][subdir] = "aegir"

; Dehydrated for LetsEncrypt.org
; libraries[dehydrated][download][type] = git
; libraries[dehydrated][download][url] = https://github.com/lukas2511/dehydrated
; libraries[dehydrated][destination] = modules/aegir/hosting_https/submodules/letsencrypt/drush/bin

; Timeago module
projects[timeago][version] = 2.3

; JQuery TimeAgo plugin
libraries[timeago][download][type] = get
libraries[timeago][download][url] = https://raw.githubusercontent.com/rmm5t/jquery-timeago/v1.5.3/jquery.timeago.js
libraries[timeago][destination] = libraries

; Bootstrap base theme
projects[bootstrap][type] = theme
projects[bootstrap][version] = 3.14

; Include devel module.
projects[devel][version] = "1"
