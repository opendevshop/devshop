core = 7.x
api = 2

; PLEASE NOTE: DevShop installs from the makefile https://github.com/opendevshop/devshop/blob/1.x/build-devmaster.make
; THIS VERSION NUMBER HAS NO EFFECT when using the devshop install process. This only affects drupal.org tarballs.
; Make sure you change Drupal core version number in the files:
;  - https://github.com/opendevshop/devshop/blob/1.x/build-devmaster.make
;  - https://github.com/opendevshop/devshop/blob/1.x/build-devmaster-dev.yml
;  - https://github.com/opendevshop/devshop/blob/1.x/build-devmaster-travis-forks.make.yml
projects[drupal][type] = core
projects[drupal][version] = 7.72
