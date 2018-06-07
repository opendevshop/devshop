;
; Loads the DevMaster install profile from drupal.org.
;
; This makefile is used by the DevShop standalone installer to build devmaster.
;

core = 7.x
api = 2

projects[drupal][type] = core
projects[drupal][version] = 7.59

; RELEASE
; Leave in place for replacement by release process.
projects[devmaster][version] = 1.32
projects[devmaster][type] = profile
projects[devmaster][download][type] = git
projects[devmaster][download][branch] = 7.x-1.x