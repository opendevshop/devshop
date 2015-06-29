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

; Change this URL to your own fork to develop devshop.
projects[devmaster][download][url] = "https://github.com/opendevshop/devmaster.git"
projects[devmaster][download][branch] = "0.3.0"
