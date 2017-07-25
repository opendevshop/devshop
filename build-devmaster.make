;
; Loads the DevMaster install profile from drupal.org.
;
; This makefile is used by the DevShop standalone installer to build devmaster.
;

core = 7.x
api = 2

projects[drupal][type] = "core"

; DEVELOPMENT MODE
; When in development, use this:
projects[devmaster][type] = "profile"
projects[devmaster][download][type] = "git"
projects[devmaster][download][url] = "http://github.com/opendevshop/devmaster.git"
projects[devmaster][download][branch] = "1.x"
projects[devmaster][download][branch] = "remove-deploy"

; RELEASE
; When releasing, lock in the devmaster version.
;projects[devmaster][download][branch] = "1.0.0"
