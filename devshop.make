core = 6.x
api = 2

projects[drupal][type] = "core"

; DevShop modules
projects[devshop_hosting][download][type] = "git"
projects[devshop_hosting][type] = "module"

; Contrib modules
projects[adminrole][type] = "module"
projects[ctools][type] = "module"
projects[hosting_drush_aliases][type] = "module"
projects[hosting_filemanager][type] = "module"
projects[hosting_logs][type] = "module"
projects[hosting_queue_runner][type] = "module"
projects[hosting_solr][type] = "module"
projects[hosting_tasks_extra][type] = "module"

; Aegir Hostmaster modules
; This includes all hostmaster contrib and libraries!
projects[hostmaster][download][type] = "git"
projects[hostmaster][download][branch] = "6.x-1.x"
projects[hostmaster][type] = "profile"


