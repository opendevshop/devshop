core = 6.x
api = 2

projects[drupal][type] = "core"
projects[drupal][version] = "6.26"

; DevShop modules
projects[devshop_hosting][download][type] = "git"
projects[devshop_hosting][type] = "module"

; DevMaster Install Profile
projects[devmaster][download][type] = "git"
projects[devmaster][type] = "profile"

; Contrib modules
projects[ctools][type] = "module"

; Aegir Hostmaster modules
; This includes all hostmaster contrib and libraries!
projects[hostmaster][download][type] = "git"
projects[hostmaster][type] = "profile"
