;
; Loads the DevMaster install profile from drupal.org.
;
; This makefile is used by the DevShop standalone installer to build devmaster.
;
; This makefile targets git repos directly so that it can be used standalone.
; It will not load the ./devmaster project from this directory, but will load
; it from the remote sub repository https://github.com/opendevshop/devmaster.git
;
; The DevShop development and testing tools, use the build-devmaster-dev.make.yml
; file instead so you can develop the project and distribution at the same time.

core = 7.x
api = 2

projects[drupal][type] = core
projects[drupal][version] = 7.73

; RELEASE
; Leave in place for replacement by release process.
projects[devmaster][version] = 1.x-dev
projects[devmaster][type] = "profile"
projects[devmaster][download][type] = "git"

###DEVELOPMENTSTART###
; This is used by the drupal.org system to create "dev" releases.
; @TODO: Consider removing the "devmaster" github repo and just use drupal.org.
; DEVELOPMENT OVERRIDE
; Removed during release process.
; This is so that different version of "devshop" (the branch) can install a different version of devmaster.
; This ensures that people that install the latest 1.x devshop get the absolute latest from github.
projects[devmaster][type] = profile
projects[devmaster][download][type] = git
projects[devmaster][download][branch] = 7.x-1.x
projects[devmaster][download][url] = "https://github.com/opendevshop/devmaster.git"
###DEVELOPMENTEND###

; CAS
; The CAS and HybridAuth libraries are not whitelisted by drupal.org, so we include them here.
libraries[cas][download][type] = "git"
libraries[cas][download][url] = "https://github.com/apereo/phpCAS"
libraries[cas][download][tag] = "1.3.8"
libraries[cas][destination] = "libraries"

; Hybrid Auth
libraries[hybridauth][download][type] = "git"
libraries[hybridauth][download][url] = "https://github.com/hybridauth/hybridauth"
libraries[hybridauth][download][tag] = "v2.16.0"
libraries[hybridauth][destination] = "libraries"

; Library: Modernizr
; @TODO: move to drupal-org-contrib.make in devmaster repo once it is whitelisted.
libraries[modernizr][download][type] = git
libraries[modernizr][download][url] = https://github.com/BrianGilbert/modernizer-navbar.git
libraries[modernizr][download][revision] = 5b89d9225320e88588f1cdc43b8b1e373fa4c60f
