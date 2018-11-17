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
projects[devmaster][version] = 1.x-dev

; DEVELOPMENT & TESTING
; When you need to test or install devshop using a devmaster branch, uncomment this.
; BE SURE TO COMMENT THIS OUT FOR RELEASE.
; projects[devmaster][type] = "profile"
; projects[devmaster][download][type] = "git"
; projects[devmaster][download][url] = "https://github.com/opendevshop/devmaster"
; projects[devmaster][download][branch] = "7.x-1.x"

; CAS
libraries[cas][download][type] = "git"
libraries[cas][download][url] = "https://github.com/apereo/phpCAS"
libraries[cas][download][tag] = "1.3.5"
libraries[cas][destination] = "libraries"

; Hybrid Auth
libraries[hybridauth][download][type] = "git"
libraries[hybridauth][download][url] = "https://github.com/hybridauth/hybridauth"
libraries[hybridauth][download][tag] = "v2.10.0"
libraries[hybridauth][destination] = "libraries"

; Intercom Module
projects[intercomio][type] = module
projects[intercomio][download][type] = git
projects[intercomio][download][branch] = composer-autoload
projects[intercomio][download][url] = "https://github.com/thinkdrop/drupal-intercomio.git"
projects[intercomio][version] = 1.x
