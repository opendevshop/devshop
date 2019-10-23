;
; This makefile is used by the DevShop standalone installer to build devmaster.
;

core = 6.x
api = 2

projects[drupal][type] = "core"

; DEVELOPMENT MODE:
; When in development, use this:
projects[devmaster][type] = "profile"
projects[devmaster][download][type] = "git"
projects[devmaster][download][url] = "git@git.drupal.org:project/devmaster.git"
projects[devmaster][download][branch] = "6.x-1.x"

; RELEASE:
; When releasing, lock in the devmaster version.
;projects[devmaster][version] = "6.x-1.0"