;
; Loads the DevMaster install profile from drupal.org.
;
; This makefile is used by the DevShop standalone installer to build devmaster.
;

core = 7.x
api = 2

projects[drupal][type] = core
projects[drupal][version] = 7.56

projects[devmaster][type] = "profile"
projects[devmaster][download][type] = "git"
projects[devmaster][download][url] = "http://github.com/opendevshop/devmaster.git"
projects[devmaster][download][branch] = "1.x"

; RELEASE
; Leave RELEASE_LINE in place for replacement by release script in RoboFile.php
;RELEASE_LINE
