core = 6.x
api = 2

; this makefile fetches the latest Aegir code from git from drupal.org
; it shouldn't really change at all apart from major upgrades, where
; the branch will change
projects[drupal][type] = "core"

; chain into hostmaster from git's 2.x branch
projects[hostmaster][type] = "profile"
projects[hostmaster][download][type] = "git"
projects[hostmaster][download][url] = "http://git.drupal.org/project/hostmaster.git"
projects[hostmaster][download][branch] = "6.x-2.x"

projects[devshop][type] = "profile"
projects[devshop][download][type] = "git"
projects[devshop][download][url] = "http://git.drupal.org/project/devshop.git"
projects[devshop][download][branch] = "6.x-1.x-aegir2"
