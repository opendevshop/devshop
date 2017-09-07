;
; Loads the DevMaster install profile from drupal.org.
;
; This makefile is used by the DevShop standalone installer to build devmaster.
;

core = 7.x
api = 2

includes[] = drupal-org-core.make
projects[devmaster][version] = 1.00-beta10