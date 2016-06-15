;
; Loads the DevMaster install profile from drupal.org.
;
; This makefile is used by the DevShop standalone installer to build devmaster.
;

core = 7.x
api = 2

projects[drupal][type] = "core"

projects[devmaster][type] = "profile"
projects[devmaster][download][type] = "git"
projects[devmaster][download][url] = "http://github.com/opendevshop/devmaster.git"

; Version of DevShop.  Set automatically in release-prep.sh
projects[devmaster][download][branch] = "1.x"
