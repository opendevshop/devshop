core = 7.x
api = 2



; Includes

; This makefile will make sure we get the development code from the Aegir
; modules instead of the tagged releases.
includes[devmaster] = drupal-org.make

;; DEVELOPMENT
;; Includes clones of all modules.
;; Comment this out for release.
includes[development] = devmaster.development.make.yml
