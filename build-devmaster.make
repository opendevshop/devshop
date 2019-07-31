;
; Loads the DevMaster install profile from drupal.org.
;
; This makefile is used by the DevShop standalone installer to build devmaster.
;

core = 7.x
api = 2

includes[] = drupal-org-core.make
projects[devmaster][type] = profile
projects[devmaster][download][type] = git
projects[devmaster][download][branch] = 7.x-1.x
