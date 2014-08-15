core = 6.x
api = 2

projects[drupal][type] = "core"

; DevShop modules
projects[devshop_hosting][subdir] = devshop
projects[devshop_hosting][version] = 6.x-1.9-beta4

; Contrib modules
projects[adminrole][subdir] = contrib
projects[ctools][subdir] = contrib
projects[hosting_drush_aliases][subdir] = contrib
projects[hosting_filemanager][subdir] = contrib
projects[hosting_logs][subdir] = contrib
projects[hosting_queue_runner][subdir] = contrib
projects[hosting_solr][subdir] = contrib
projects[hosting_tasks_extra][subdir] = contrib

; Aegir Hostmaster modules
; This includes all hostmaster contrib and libraries!

projects[hostmaster][type] = "profile"
projects[hostmaster][version] = "1.12"
